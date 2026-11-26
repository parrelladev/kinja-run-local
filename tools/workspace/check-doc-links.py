"""Confere os links relativos de todos os arquivos Markdown versionados.

Uso, na raiz do projeto:  python tools/workspace/check-doc-links.py
Sai com código 1 se algum link apontar para um caminho inexistente.
Links externos (http/https/mailto) e âncoras puras (#...) são ignorados;
a âncora depois de um caminho não é conferida, só o caminho.
"""
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
LINK = re.compile(r"\]\(([^)\s]+)\)")
SKIP = re.compile(r"^(https?:|mailto:|#)")


def tracked_markdown():
    out = subprocess.run(["git", "ls-files", "*.md"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
    return [ROOT / line for line in out.splitlines() if line]


def main():
    broken = []
    for md in tracked_markdown():
        text = md.read_text(encoding="utf-8")
        for target in LINK.findall(text):
            if SKIP.match(target):
                continue
            path = (md.parent / target.split("#", 1)[0]).resolve()
            if not path.exists():
                broken.append(f"{md.relative_to(ROOT).as_posix()}: {target}")
    for line in broken:
        print("QUEBRADO", line)
    print(f"{len(broken)} link(s) quebrado(s) em {len(tracked_markdown())} arquivo(s) Markdown.")
    return 1 if broken else 0


if __name__ == "__main__":
    sys.exit(main())
