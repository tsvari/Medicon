import os
from collections import defaultdict

sizes = defaultdict(lambda: [0, 0])

for root, dirs, files in os.walk('.'):
    if '.git' in dirs: dirs.remove('.git')
    if 'build' in dirs: dirs.remove('build')
    if '_scratch' in dirs: dirs.remove('_scratch')
    if '.git' in root or 'build' in root or '_scratch' in root:
        continue
    for f in files:
        if any(f.endswith(e) for e in ('.h','.hpp','.cpp','.ui','.proto','.cmake','.txt','.xml','.json','.qrc')):
            fp = os.path.join(root, f)
            try:
                with open(fp, 'rb') as fh:
                    lines = sum(1 for _ in fh)
            except:
                continue
            parts = [p for p in fp.replace(os.sep, '/').split('/') if p]
            top = parts[0] if len(parts) > 1 else '.'
            sizes[top][0] += 1
            sizes[top][1] += lines

print("{:<25} {:>8} {:>10}".format("Dir", "Files", "Lines"))
print("-" * 45)
for k in sorted(sizes.keys()):
    print("{:<25} {:>8} {:>10}".format(k, sizes[k][0], sizes[k][1]))
