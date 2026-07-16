"""Count actual project source code including test files."""
import os
from collections import defaultdict

EXCLUDE_DIRS = {'.git', 'build', '3party', '_scratch', '.claude', '.qtcreator'}
SRC_EXTENSIONS = {'.h', '.hpp', '.cpp', '.ui', '.proto', '.cmake', '.xml', '.json', '.qrc','.txt','.py'}

counts = defaultdict(lambda: {'files': 0, 'lines': 0})

for root, dirs, files in os.walk('.'):
    dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS and not d.startswith('.')]
    parts = set(root.replace(os.sep, '/').split('/'))
    if EXCLUDE_DIRS & parts:
        continue
    
    for f in files:
        ext = os.path.splitext(f)[1].lower()
        if ext not in SRC_EXTENSIONS:
            continue
        
        fp = os.path.join(root, f)
        try:
            with open(fp, 'rb') as fh:
                lines = sum(1 for _ in fh)
        except:
            continue
        
        rel = os.path.relpath(fp, '.').replace(os.sep, '/')
        comps = rel.split('/')
        
        # Skip user-specific files and build artifacts
        if 'CMakeLists.txt.user' in rel:
            continue
        if '.user' in f:
            continue
        
        if len(comps) >= 3 and comps[1] == 'source':
            cat = f"{comps[0]}/source/{comps[2]}"
        elif len(comps) >= 2:
            cat = f"{comps[0]}/{comps[1]}"
        else:
            cat = 'root'
        
        counts[cat]['files'] += 1
        counts[cat]['lines'] += lines

print(f"{'Category':<45} {'Files':>6} {'Lines':>10}")
print('-' * 65)
total_files = 0
total_lines = 0
for cat in sorted(counts.keys()):
    v = counts[cat]
    print(f"{cat:<45} {v['files']:>6} {v['lines']:>10,}")
    total_files += v['files']
    total_lines += v['lines']
print('-' * 65)
print(f"{'TOTAL':<45} {total_files:>6} {total_lines:>10,}")

# Also show by top-level dir
print("\n\n=== BY TOP-LEVEL DIRECTORY ===")
top = defaultdict(lambda: [0,0])
for cat in counts:
    top_level = cat.split('/')[0]
    top[top_level][0] += counts[cat]['files']
    top[top_level][1] += counts[cat]['lines']
print(f"{'Dir':<20} {'Files':>6} {'Lines':>10}")
print('-' * 40)
for k in sorted(top.keys()):
    print(f"{k:<20} {top[k][0]:>6} {top[k][1]:>10,}")
