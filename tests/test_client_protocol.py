"""Protocol tests using actual executables and a local TCP peer (standard library only)."""
import os
from pathlib import Path
import select
import signal
import socket
import subprocess
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]


def free_port(kind=socket.SOCK_DGRAM):
    for port in range(7800, 9999):
        with socket.socket(socket.AF_INET6, kind) as probe:
            try:
                probe.bind(("::", port))
                return port
            except OSError:
                pass
    raise RuntimeError("No available test port below 9999")


class Client:
    def __init__(self, port, udp_port=None):
        args = [str(ROOT / "client_exe"), "-v", "127.0.0.1", str(port)]
        if udp_port is not None:
            args.append(str(udp_port))
        self.proc = subprocess.Popen(args, cwd=ROOT, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        self.output = b""
        self.history = b""
        self.wait_for(b"> ")

    def command(self, text):
        self.proc.stdin.write(text.encode() + b"\n")
        self.proc.stdin.flush()

    def wait_for(self, marker, timeout=3):
        deadline = time.monotonic() + timeout
        while marker not in self.output:
            remaining = deadline - time.monotonic()
            if remaining <= 0 or not select.select([self.proc.stdout], [], [], remaining)[0]:
                raise AssertionError(f"Missing {marker!r}; output: {self.history!r}")
            chunk = os.read(self.proc.stdout.fileno(), 4096)
            if not chunk:
                raise AssertionError(f"Client exited before {marker!r}: {self.history!r}")
            self.output += chunk
            self.history += chunk
        end = self.output.index(marker) + len(marker)
        result, self.output = self.output[:end], self.output[end:]
        return result

    def close(self):
        if self.proc.poll() is None:
            self.proc.terminate()
        self.proc.wait(timeout=3)
        self.proc.stdin.close()
        self.proc.stdout.close()


class ProtocolTests(unittest.TestCase):
    def setUp(self):
        self.listener = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        self.listener.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
        self.listener.bind(("::", free_port(socket.SOCK_STREAM)))
        self.listener.listen(1)
        self.listener.settimeout(3)
        self.addCleanup(self.listener.close)
        self.client = Client(self.listener.getsockname()[1])
        self.addCleanup(self.client.close)
        self.peer, _ = self.listener.accept()
        self.peer.settimeout(3)
        self.addCleanup(self.peer.close)
        self.udp_port = free_port()

    def expect_wire(self, expected):
        received = b""
        while len(received) < len(expected):
            chunk = self.peer.recv(len(expected) - len(received))
            self.assertTrue(chunk, "Client closed the socket")
            received += chunk
        self.assertEqual(received, expected)

    def expect_no_wire(self):
        self.peer.settimeout(0.1)
        try:
            with self.assertRaises(socket.timeout):
                self.peer.recv(1024)
        finally:
            self.peer.settimeout(3)

    def authenticate(self, password=1):
        self.client.command(f"REGIS bob12345 {self.udp_port} {password}")
        self.expect_wire(f"REGIS bob12345 {self.udp_port:04d} ".encode()
                         + password.to_bytes(2, "little") + b"+++")
        self.peer.sendall(b"WELCO+++")
        self.client.wait_for(b"WELCO: autenticato")

    def test_validation_and_retry(self):
        cases = [
            ("LIST?", b"Prima autenticati"),
            (f"REGIS bob123456 {self.udp_port} 1", b"Id non valido"),
            (f"REGIS bob1234 {self.udp_port} 1", b"Id non valido"),
            (f"REGIS bob1234! {self.udp_port} 1", b"Id non valido"),
            (f"REGIS {'a' * 400} {self.udp_port} 1", b"Riga troppo lunga"),
            ("REGIS bob12345 65537 1", b"Porta UDP non valida"),
            (f"REGIS bob12345 {self.udp_port} 65536", b"Password non valida"),
            (f"REGIS bob12345 {self.udp_port} 4294967297", b"Password non valida"),
            (f"REGIS bob12345 {self.udp_port} 1 extra", b"Uso: REGIS"),
        ]
        for command, marker in cases:
            with self.subTest(command=command):
                self.client.command(command)
                self.client.wait_for(marker)
                self.expect_no_wire()
        self.authenticate(0x2B2B)
        self.client.command("OKIRF")
        self.client.wait_for(b"Nessuna richiesta")
        self.expect_no_wire()

    def test_zero_password_and_fragmented_reply_with_udp(self):
        self.client.command(f"REGIS bob12345 {self.udp_port} 0")
        self.expect_wire(f"REGIS bob12345 {self.udp_port:04d} ".encode() + b"\0\0+++")
        self.peer.sendall(b"WEL")
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp:
            udp.sendto(b"3A1", ("127.0.0.1", self.udp_port))
        self.client.wait_for(b"26 flussi non consultati")
        self.client.command("HELP")
        self.client.wait_for(b"HELP                        mostra")
        self.peer.sendall(b"CO+++")
        self.client.wait_for(b"WELCO: autenticato")
        self.client.command("CONSU")
        self.expect_wire(b"CONSU+++")
        self.peer.sendall(b"NOCON+++")
        self.client.wait_for(b"NOCON: nessun flusso")

    def test_friend_accept_and_reject_sequence(self):
        self.authenticate()
        for reply in ["OKIRF", "NOKRF"]:
            self.client.command("CONSU")
            self.expect_wire(b"CONSU+++")
            self.peer.sendall(b"EIRF> alice123+++")
            self.client.wait_for(b"Rispondi con OKIRF o NOKRF")
            for wrong in ["CONSU", "acc", "rej"]:
                self.client.command(wrong)
                self.client.wait_for(b"Rispondi prima alla richiesta")
                self.expect_no_wire()
            self.client.command(reply)
            self.expect_wire(reply.encode() + b"+++")
            self.client.command("CONSU")
            self.client.wait_for(b"Attendi la risposta")
            self.expect_no_wire()
            self.peer.sendall(b"ACKRF+++")
            self.client.wait_for(b"ACKRF: risposta")

    def test_list_and_all_consultation_types(self):
        self.authenticate()
        self.client.command("LIST?")
        self.expect_wire(b"LIST?+++")
        self.peer.sendall(b"RLIST 002+++LINUM alice123+++LINUM bob12345+++")
        self.client.wait_for(b"LINUM: bob12345")
        for response, marker in [
            (b"SSEM> alice123 hello world+++", b"SSEM> da alice123: hello world"),
            (b"OOLF> alice123 flood message+++", b"OOLF> da alice123: flood message"),
            (b"FRIEN alice123+++", b"ha accettato"),
            (b"NOFRI alice123+++", b"ha rifiutato"),
            (b"NOCON+++", b"NOCON: nessun flusso"),
        ]:
            self.client.command("CONSU")
            self.expect_wire(b"CONSU+++")
            self.peer.sendall(response)
            self.client.wait_for(marker)

    def test_message_limits_and_request_names(self):
        self.authenticate()
        for command in ["MESS? alice123 " + "x" * 201, "FLOO? bad+++message"]:
            self.client.command(command)
            self.client.wait_for(b"Messaggio non valido")
            self.expect_no_wire()
        for command, wire, response, marker in [
            ("FRIE? alice123", b"FRIE? alice123+++", b"FRIE>+++", b"FRIE>: richiesta"),
            ("MESS? alice123 " + "x" * 200, b"MESS? alice123 " + b"x" * 200 + b"+++", b"MESS>+++", b"MESS>: messaggio"),
            ("FLOO? hello world", b"FLOO? hello world+++", b"FLOO>+++", b"FLOO>: flood"),
        ]:
            self.client.command(command)
            self.expect_wire(wire)
            self.peer.sendall(response)
            self.client.wait_for(marker)
        for old_alias in ["acc", "rej", "quit"]:
            self.client.command(old_alias)
            self.client.wait_for(b"Comando sconosciuto")
            self.expect_no_wire()

    def test_quit_waits_for_gobye(self):
        self.authenticate()
        self.client.command("IQUIT")
        self.expect_wire(b"IQUIT+++")
        self.assertIsNone(self.client.proc.poll())
        self.peer.sendall(b"GOBYE+++")
        self.client.wait_for(b"GOBYE: disconnessione confermata")
        self.assertEqual(self.client.proc.wait(timeout=3), 0)

    def test_sigint_uses_quit_handshake(self):
        self.authenticate()
        self.client.proc.send_signal(signal.SIGINT)
        self.expect_wire(b"IQUIT+++")
        self.peer.sendall(b"GOBYE+++")
        self.assertEqual(self.client.proc.wait(timeout=3), 0)

    def test_malformed_server_reply_closes_cleanly(self):
        self.authenticate()
        self.client.command("CONSU")
        self.expect_wire(b"CONSU+++")
        self.peer.sendall(b"EIRF> too_long_id+++")
        self.client.wait_for(b"Risposta TCP malformata")
        self.client.proc.wait(timeout=3)


if __name__ == "__main__":
    unittest.main(verbosity=2)
