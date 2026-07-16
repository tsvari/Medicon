"""Count actual project source code (excluding 3party/, build/, .git/)."""
import os
from collections import defaultdict

EXCLUDE_DIRS = {'.git', 'build', '3party', '_scratch', '.claude'}
EXCLUDE_FILES_STARTING_WITH = {'.'}
SRC_EXTENSIONS = {'.h', '.hpp', '.cpp', '.ui', '.proto', '.cmake', '.xml', '.json', '.qrc'}

counts = defaultdict(lambda: {'files': 0, 'lines': 0})

for root, dirs, files in os.walk('.'):
    # Skip excluded dirs
    dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS and not d.startswith('.')]
    # Also skip paths containing excluded dirs
    parts = set(root.replace(os.sep, '/').split('/'))
    if EXCLUDE_DIRS & parts:
        continue
    
    for f in files:
        ext = os.path.splitext(f)[1].lower()
        if ext not in SRC_EXTENSIONS:
            continue
        if any(f.startswith(p) for p in EXCLUDE_FILES_STARTING_WITH):
            continue
        
        fp = os.path.join(root, f)
        try:
            with open(fp, 'rb') as fh:
                lines = sum(1 for _ in fh)
        except:
            continue
        
        # Determine category (first 1-2 path components)
        rel = os.path.relpath(fp, '.').replace(os.sep, '/')
        comps = rel.split('/')
        if len(comps) >= 2:
            if comps[1] == 'source' and len(comps) >= 3:
                cat = f"{comps[0]}/{comps[1]}"
            else:
                cat = comps[0]
        else:
            cat = 'root'
        
        counts[cat]['files'] += 1
        counts[cat]['lines'] += lines

print(f"{'Category':<30} {'Files':>8} {'Lines':>10}")
print('-' * 50)
total_files = 0
total_lines = 0
for cat in sorted(counts.keys()):
    v = counts[cat]
    print(f"{cat:<30} {v['files']:>8} {v['lines']:>10,}")
    total_files += v['files']
    total_lines += v['lines']
print('-' * 50)
print(f"{'TOTAL':<30} {total_files:>8} {total_lines:>10,}")
