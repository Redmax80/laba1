"""Сохранить проверенный фактический вывод контрольных примеров."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SHARED = [
    ("MPUSH scores 8", "8"),
    ("MPUSH scores 12", "12"),
    ("MINSERT scores 1 10", "10"),
    ("PRINT scores", "8 10 12"),
    ("FINSERTHEAD route A", "A"),
    ("FINSERTTAIL route B", "B"),
    ("FINSERTAFTER route A C", "C"),
    ("FREAD route", "A C B"),
    ("FDELBEFORE route B", "C"),
    ("FREAD route REVERSE", "B A"),
    ("LINSERTHEAD history A", "A"),
    ("LINSERTTAIL history B", "B"),
    ("LINSERTBEFORE history B C", "C"),
    ("LREAD history REVERSE", "B C A"),
    ("SPUSH undo A", "A"),
    ("SPUSH undo B", "B"),
    ("SPOP undo", "B"),
    ("SREAD undo", "A"),
    ("QPUSH tasks A", "A"),
    ("QPUSH tasks B", "B"),
    ("QPOP tasks", "A"),
    ("QREAD tasks", "B"),
]
TREE = [(f"TINSERT tree {key}", str(key)) for key in (40, 10, 70, 20, 60, 10)] + [
    ("TREAD tree", "ROOT: 40\n  L: 10\n    L: 20\n    R: 60\n  R: 70\n    L: 10"),
    ("TISCOMPLETE tree", "TRUE"),
    ("ISMEMBER tree 60", "TRUE"),
    ("TGET tree 999", "FALSE"),
]


def main():
    combined = []
    for language in ("cpp", "go"):
        lines = [f"Фактический вывод dbms-{language}: отдельный процесс на каждый запрос", ""]
        with tempfile.TemporaryDirectory() as directory:
            data = Path(directory) / "example.data"
            for query, expected in SHARED + (TREE if language == "cpp" else []):
                result = subprocess.run([str(ROOT / f"build/dbms-{language}"), "--file", str(data), "--query", query],
                                        capture_output=True, text=True, timeout=10)
                if result.returncode != 0 or result.stdout.rstrip("\n") != expected:
                    raise RuntimeError((language, query, expected, result.stdout, result.stderr))
                lines.extend([f"$ --query '{query}'", result.stdout.rstrip("\n")])
        output = "\n".join(lines) + "\n"
        (ROOT / f"docs/evidence-{language}.txt").write_text(output)
        combined.append(output)
    (ROOT / "docs/evidence.txt").write_text("\n".join(combined))
    print("Проверены и сохранены 54 контрольных запуска: C++ 32, Go 22.")


if __name__ == "__main__":
    main()
