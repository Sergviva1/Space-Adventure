"""Составлен аудит ресурсов в достижимой истории Git без изменения репозитория."""

import json
import pathlib
import subprocess


def git(*args):
    return subprocess.check_output(["git", *args])


root = pathlib.Path(__file__).resolve().parent.parent
# Скрипт запускался из корня, чтобы исключить аудит другого репозитория.
if pathlib.Path(git("rev-parse", "--show-toplevel").decode().strip()).resolve() != root:
    raise SystemExit("Run from this repository")


def tree(commit):
    for entry in git("ls-tree", "-r", "-z", commit).split(b"\0"):
        if not entry:
            continue
        meta, path = entry.split(b"\t", 1)
        _, kind, oid = meta.decode().split()
        if kind == "blob":
            yield path.decode("utf-8"), oid


extensions = {".png", ".jpg", ".jpeg", ".ogg", ".wav", ".mp3", ".ttf"}
head = dict(tree("HEAD"))
current = {oid for path, oid in head.items() if path.startswith("assets/")}
commits = git("rev-list", "--all", "--reverse", "--topo-order").decode().splitlines()
blobs = {}
all_paths = {}
for commit in commits:
    for path, oid in tree(commit):
        all_paths.setdefault(oid, set()).add(path)
        if path.startswith(("assets/", "coding/assets/")) and pathlib.PurePosixPath(path).suffix.lower() in extensions:
            item = blobs.setdefault(oid, {"blob": oid, "paths": set(), "first_seen_commit": commit})
            item["paths"].add(path)

candidates = []
retained = []
for oid, item in sorted(blobs.items()):
    item["paths"] = sorted(item["paths"])
    item["all_paths_in_history"] = sorted(all_paths[oid])
    item["bytes"] = int(git("cat-file", "-s", oid))
    item["present_in_head_assets"] = oid in current
    (retained if oid in current else candidates).append(item)

refs = git("for-each-ref", "--format=%(refname) %(objectname)").decode().splitlines()
report = {
    "scope": "git rev-list --all; reachable commits only, no fetch, no reflogs or unreachable objects",
    "head": git("rev-parse", "HEAD").decode().strip(),
    "refs": refs,
    "commits_scanned": len(commits),
    "classification": "Historical media blobs absent from HEAD assets are review candidates, not an automatic legal determination.",
    "candidate_count": len(candidates),
    "candidate_bytes": sum(x["bytes"] for x in candidates),
    "candidates": candidates,
    "retained_current_assets": retained,
}
output = root / "docs" / "asset-history-audit.json"
output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(f"Commits: {len(commits)}; candidate blobs: {len(candidates)}; bytes: {report['candidate_bytes']}")
print(f"Current media blobs retained: {len(retained)}")
for item in candidates:
    print(item["blob"], item["bytes"], ", ".join(item["paths"]))
print(output)
