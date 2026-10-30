"""Проверки публичного CLI: каждый запрос запускает отдельный процесс."""
from pathlib import Path
import os
import random
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
CPP = ROOT / "build/dbms-cpp"
GO = ROOT / "build/dbms-go"


class Commands:
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / "sample.data"

    def query(self, text, expected=None, success=True, binary=None):
        before = self.path.read_bytes() if self.path.exists() else None
        result = subprocess.run(
            [str(binary or self.binary), "--file", str(self.path), "--query", text],
            text=True, capture_output=True, timeout=10,
        )
        self.assertEqual(result.returncode, 0 if success else 1, (text, result.stderr))
        if expected is not None:
            self.assertEqual(result.stdout.rstrip("\n"), expected, text)
        if not success:
            self.assertTrue(result.stderr, text)
            after = self.path.read_bytes() if self.path.exists() else None
            self.assertEqual(before, after, "failed query changed the data file")
        return result.stdout.rstrip("\n")

    def test_array_operations_and_growth(self):
        self.query("MCREATE m", "OK")
        for i in range(20):
            self.query(f"MPUSH m v{i}", f"v{i}")
        self.query("MLEN m", "20")
        self.query("MINSERT m 0 start", "start")
        self.query("MINSERT m 21 finish", "finish")
        self.query("MINSERT m 5 middle", "middle")
        self.query("MSET m 5 updated", "updated")
        self.query("MGET m 5", "updated")
        self.query("MDEL m 5", "updated")
        self.query("MDEL m 0", "start")
        self.query("MDEL m 20", "finish")
        self.query("PRINT m", " ".join(f"v{i}" for i in range(20)))
        for index in ("-1", "-0", "+1", "1.5", "1x", "2147483648", "20"):
            self.query(f"MGET m {index}", success=False)
        self.query("MINSERT m 21 x", success=False)

    def test_lists_all_operations(self):
        for prefix in ("F", "L"):
            name = prefix.lower()
            def q(action, output=None, success=True):
                return self.query(prefix + action.split()[0] + " " + name +
                                  (" " + " ".join(action.split()[1:]) if len(action.split()) > 1 else ""),
                                  output, success)
            q("CREATE", "OK")
            q("READ", "(empty)")
            q("INSERTTAIL B", "B")
            q("INSERTHEAD A", "A")
            q("PUSH D", "D")
            q("INSERTBEFORE D C", "C")
            q("INSERTAFTER D E", "E")
            q("READ FORWARD", "A B C D E")
            q("READ REVERSE", "E D C B A")
            q("FIND C", "2")
            q("FIND missing", "-1")
            q("GET C", "C")
            q("DELBEFORE A", success=False)
            q("DELAFTER E", success=False)
            q("DELBEFORE D", "C")
            q("DELAFTER D", "E")
            q("DEL B", "B")
            q("REMOVE D", "D")
            q("DELTAIL", "A")
            q("READ", "(empty)")
            q("DELHEAD", success=False)
            q("DELTAIL", success=False)
            q("INSERTHEAD X", "X")
            q("DELHEAD", "X")
            q("PUSH A", "A")
            q("PUSH A", "A")
            q("INSERTBEFORE A Z", "Z")
            q("READ", "Z A A")
            q("DEL A", "A")
            q("READ", "Z A")
            q("GET missing", success=False)
            q("INSERTAFTER missing X", success=False)
            q("READ SIDEWAYS", success=False)

    def test_stack_and_queue(self):
        for prefix, first, second in (("S", "B", "A"), ("Q", "A", "B")):
            self.query(f"{prefix}CREATE x{prefix}", "OK")
            self.query(f"{prefix}POP x{prefix}", success=False)
            self.query(f"{prefix}GET x{prefix}", success=False)
            self.query(f"{prefix}PUSH x{prefix} A", "A")
            self.query(f"{prefix}PUSH x{prefix} B", "B")
            self.query(f"{prefix}READ x{prefix}", f"{first} {second}")
            self.query(f"{prefix}GET x{prefix}", first)
            self.query(f"{prefix}POP x{prefix}", first)
            self.query(f"{prefix}POP x{prefix}", second)
            self.query(f"{prefix}READ x{prefix}", "(empty)")
            self.query(f"{prefix}PUSH x{prefix} again", "again")
            self.query(f"{prefix}POP x{prefix}", "again")

    def test_validation_and_type_errors(self):
        for query in ("", "UNKNOWN a", "MPUSH", "MPUSH a", "MPUSH a b c", "FINSERTBEFORE a b", "PRINT absent"):
            self.query(query, success=False)
        self.assertFalse(self.path.exists())
        self.query("MPUSH a привет", "привет")
        for query in ("MCREATE a", "SPUSH a x", "MGET missing 0", "MUNKNOWN a", "HELP extra", "MLEN a extra"):
            self.query(query, success=False)
        self.query("PRINT a", "привет")

    def test_read_commands_do_not_rewrite(self):
        self.query("MPUSH m value", "value")
        os.utime(self.path, ns=(1000000000, 1000000000))
        before = self.path.stat().st_mtime_ns
        self.query("PRINT m", "value")
        self.query("MGET m 0", "value")
        self.query("HELP")
        self.assertEqual(before, self.path.stat().st_mtime_ns)

    def test_corrupt_files_are_not_overwritten(self):
        for content in ("", "-1\n", "1\nZ x 0\n", "1\nMM x 0\n", "2\nM x 0\nM x 0\n", "1\nM x 2\none\n", "0\ntrailing", "1\nM x -1\n"):
            self.path.write_text(content)
            self.query("MPUSH new x", success=False)

    def test_cli_options(self):
        for args in ([], ["--file", str(self.path)], ["--file", "x", "--file", "y"],
                     ["--unknown", "x", "--query", "HELP"], ["--file", str(self.path), "--query"]):
            result = subprocess.run([str(self.binary), *args], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 1)
        self.assertFalse(self.path.exists())
        result = subprocess.run([str(self.binary), "--query", "HELP", "--file", str(self.path)], capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_randomized_array_commands(self):
        self.query("MCREATE m", "OK")
        model = []
        rng = random.Random(2026)
        for step in range(60):
            if not model or rng.randrange(2) == 0:
                index, value = rng.randrange(len(model)+1), f"item{step}"
                self.query(f"MINSERT m {index} {value}", value)
                model.insert(index, value)
            else:
                index = rng.randrange(len(model))
                self.query(f"MDEL m {index}", model.pop(index))
            self.query("PRINT m", " ".join(model) or "(empty)")

    def test_failed_save(self):
        self.path = self.path.parent / "absent" / "sample.data"
        self.query("MPUSH m value", success=False)
        self.assertFalse(self.path.exists())

    def test_existing_file_survives_write_failure(self):
        self.query("MPUSH m original", "original")
        self.path.parent.chmod(0o500)
        try:
            self.query("MPUSH m extra", success=False)
        finally:
            self.path.parent.chmod(0o700)
        self.query("PRINT m", "original")

    def test_randomized_list_commands(self):
        rng = random.Random(2027)
        for prefix in ("F", "L"):
            model = []
            self.query(f"{prefix}CREATE {prefix}", "OK")
            for step in range(60):
                action = rng.choice(("INSERTHEAD", "INSERTTAIL", "INSERTBEFORE", "INSERTAFTER", "DEL", "DELBEFORE", "DELAFTER"))
                if not model:
                    action = "INSERTHEAD"
                value = f"item{step}"
                if action in ("INSERTHEAD", "INSERTTAIL"):
                    self.query(f"{prefix}{action} {prefix} {value}", value)
                    model.insert(0 if action == "INSERTHEAD" else len(model), value)
                else:
                    index = rng.randrange(len(model))
                    target = model[index]
                    if action.startswith("INSERT"):
                        self.query(f"{prefix}{action} {prefix} {target} {value}", value)
                        model.insert(index + (action == "INSERTAFTER"), value)
                    else:
                        removed_index = index + (1 if action == "DELAFTER" else -1 if action == "DELBEFORE" else 0)
                        if removed_index < 0 or removed_index >= len(model):
                            self.query(f"{prefix}{action} {prefix} {target}", success=False)
                        else:
                            expected = model.pop(removed_index)
                            self.query(f"{prefix}{action} {prefix} {target}", expected)
                self.query(f"PRINT {prefix}", " ".join(model) or "(empty)")
                self.query(f"{prefix}READ {prefix} REVERSE", " ".join(reversed(model)) or "(empty)")


class CppCommands(Commands, unittest.TestCase):
    binary = CPP


class GoCommands(Commands, unittest.TestCase):
    binary = GO


class TreeAndInteroperability(unittest.TestCase):
    binary = CPP
    setUp = Commands.setUp
    query = Commands.query

    def test_complete_tree_shape_and_roundtrip(self):
        self.query("TCREATE t", "OK")
        self.query("TISCOMPLETE t", "TRUE")
        self.query("TREAD t", "(empty)")
        for key in (40, 10, 70, 20, 60, 10):
            self.query(f"TINSERT t {key}", str(key))
        self.query("TREAD t", "ROOT: 40\n  L: 10\n    L: 20\n    R: 60\n  R: 70\n    L: 10")
        self.query("TGET t 60", "TRUE")
        self.query("ISMEMBER t 999", "FALSE")
        self.query("TISCOMPLETE t", "TRUE")
        self.query("TDEL t 40", success=False)
        for key in ("x", "+1", "2147483648", "-2147483649"):
            self.query(f"TINSERT t {key}", success=False)
        self.query("TINSERT t -2147483648", "-2147483648")
        self.query("TINSERT t 2147483647", "2147483647")
        self.query("TISCOMPLETE t", "TRUE")

    def test_shared_file_and_tree_preservation(self):
        for query, output in (("MPUSH m a", "a"), ("FPUSH f b", "b"), ("LPUSH l c", "c"),
                              ("SPUSH s low", "low"), ("SPUSH s top", "top"), ("QPUSH q first", "first"),
                              ("TINSERT t 30", "30"), ("TINSERT t 5", "5")):
            self.query(query, output)
        self.query("MINSERT m 1 z", "z", binary=GO)
        self.query("SPOP s", "top", binary=GO)
        self.query("QPUSH q second", "second", binary=GO)
        self.query("TINSERT t 1", success=False, binary=GO)
        self.query("PRINT t", success=False, binary=GO)
        self.query("PRINT m", "a z")
        self.query("PRINT f", "b")
        self.query("PRINT l", "c")
        self.query("PRINT s", "low")
        self.query("PRINT q", "first second")
        self.query("PRINT t", "ROOT: 30\n  L: 5")
        self.query("TISCOMPLETE t", "TRUE")

    def test_go_file_is_readable_by_cpp(self):
        self.query("SPUSH s 1", "1", binary=GO)
        self.query("SPUSH s 2", "2", binary=GO)
        self.query("SPOP s", "2")
        self.query("SPOP s", "1", binary=GO)


if __name__ == "__main__":
    unittest.main()
