"""Собрать DOCX и изображения реального вывода, сохранённого capture_examples.py.

Дополнительные зависимости только для отчёта: requirements-report.txt.
"""
from pathlib import Path
import re

from docx import Document
from docx.shared import Inches, Pt
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / "docs"


def make_evidence_image(language):
    text = (DOCS / f"evidence-{language}.txt").read_text()
    lines = [f"dbms-{language}: фактический вывод контрольных запросов", ""]
    selected = False
    for line in text.splitlines():
        if line.startswith("$ --query"):
            command = line.split("'", 2)[1].split()[0]
            selected = command in {"PRINT", "FREAD", "FDELBEFORE", "LREAD", "SPOP", "SREAD", "QPOP", "QREAD", "TREAD", "TISCOMPLETE", "ISMEMBER", "TGET"}
        if selected:
            lines.append(line)
    font = ImageFont.truetype("DejaVuSansMono.ttf", 20)
    width = max(int(font.getlength(line)) for line in lines) + 48
    image = Image.new("RGB", (width, len(lines) * 28 + 48), "#14202c")
    draw = ImageDraw.Draw(image)
    for index, line in enumerate(lines):
        draw.text((24, 24 + index * 28), line, font=font,
                  fill="#9bdbfa" if line.startswith("$") else "#edf4fa")
    path = DOCS / f"evidence-{language}.png"
    image.save(path)
    return path


def plain(text):
    return text.replace("**", "").replace("`", "")


def main():
    images = [make_evidence_image(language) for language in ("cpp", "go")]
    document = Document()
    normal = document.styles["Normal"]
    normal.font.name = "Times New Roman"
    normal.font.size = Pt(12)
    normal.paragraph_format.space_after = Pt(5)
    section = document.sections[0]
    section.top_margin = section.bottom_margin = Inches(0.65)
    section.left_margin = section.right_margin = Inches(0.7)
    code = False
    code_lines = []
    pending = []
    table = None

    def flush():
        if pending:
            document.add_paragraph(plain(" ".join(pending)))
            pending.clear()

    for line in (DOCS / "REPORT.md").read_text().splitlines():
        if line.startswith("```"):
            flush()
            if code:
                paragraph = document.add_paragraph()
                run = paragraph.add_run("\n".join(code_lines))
                run.font.name = "DejaVu Sans Mono"
                run.font.size = Pt(9)
                code_lines.clear()
            code = not code
            continue
        if code:
            code_lines.append(line)
            continue
        if line.startswith("|"):
            flush()
            cells = [plain(cell.strip()) for cell in line.strip("|").split("|")]
            if all(re.fullmatch(r"[-: ]+", cell) for cell in cells):
                continue
            if table is None:
                table = document.add_table(rows=0, cols=len(cells))
                table.style = "Table Grid"
            row = table.add_row()
            for cell, content in zip(row.cells, cells):
                cell.text = content
                for paragraph in cell.paragraphs:
                    for run in paragraph.runs:
                        run.font.size = Pt(9)
            continue
        table = None
        if not line:
            flush()
        elif line.startswith("#"):
            flush()
            depth = len(line) - len(line.lstrip("#"))
            document.add_heading(plain(line.lstrip("# ")), level=min(depth, 3))
        elif line.startswith("- "):
            flush()
            document.add_paragraph(plain(line[2:]), style="List Bullet")
        elif line.endswith("  "):
            flush()
            document.add_paragraph(plain(line.strip()))
        else:
            pending.append(line)
    flush()
    document.add_page_break()
    document.add_heading("Приложение: визуальное подтверждение запусков", level=1)
    document.add_paragraph("Изображения построены по фактическому выводу программ; полный журнал находится в docs/evidence.txt. Это визуализация вывода, а не снимок рабочего стола.")
    for language, path in zip(("C++", "Go"), images):
        document.add_heading(language, level=2)
        # Сохраняем читаемый размер и вписываем высокое изображение в страницу.
        with Image.open(path) as image:
            width = min(6.6, 8.0 * image.width / image.height)
        document.add_picture(str(path), width=Inches(width))
    output = ROOT / "Отчёт_ЛР1.docx"
    document.save(output)
    print("Сформированы DOCX и два изображения фактического вывода.")


if __name__ == "__main__":
    main()
