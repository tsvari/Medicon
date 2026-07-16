import os, sys
from collections import defaultdict

sizes = defaultdict(lambda: {'files': 0, 'lines': 0, 'dirs': set()})
for root, dirs, files in os.walk('.'):
    if '.git' in dirs: dirs.remove('.git')
    if 'build' in dirs: dirs.remove('build')
    if '_scratch' in dirs: dirs.remove('_scratch')
    if '.git' in root or 'build' in root or '_scratch' in root:
        continue
    for f in files:
        if f.endswith(('.h', '.hpp', '.cpp', '.ui', '.proto', '.cmake', '.txt', '.xml', '.json', '.qrc')):
            fp = os.path.join(root, f)
            try:
                with open(fp, 'rb') as fh:
                    lines = sum(1 for _ in fh)
            except:
                continue
            parts = fp.replace(os.sep, '/').split('/')
            if len(parts) >= 2:
                top = parts[0]
                sizes[top]['files'] += 1
                sizes[top]['lines'] += lines
                sizes[top]['dirs'].add(os.path.dirname(fp))

print(f"{'Category':<20} {'Files':>8} {'Lines':>10}")
print('-'*40)
for k in sorted(sizes.keys()):
    v = sizes[k]
    print(f"{k:<20} {v['files']:>8} {v['lines']:>10}")
