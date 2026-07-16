import sqlite3, os

db_path = os.path.join('ToolsForAI', 'source_graph.db')
conn = sqlite3.connect(db_path)
c = conn.cursor()

# Check project categories
print("=== PROJECT CATEGORIES IN FILES TABLE ===")
c.execute("SELECT DISTINCT project FROM files ORDER BY project")
for r in c.fetchall():
    print(f"  '{r[0]}'")

# Count files by project
print("\n=== FILES BY PROJECT ===")
c.execute("SELECT project, COUNT(*) FROM files GROUP BY project ORDER BY COUNT(*) DESC")
for r in c.fetchall():
    print(f"  {r[0]}: {r[1]}")

# Count lines by project
print("\n=== LINES BY PROJECT ===")
c.execute("SELECT fl.project, SUM(fl.line_count) FROM file_lines fl GROUP BY fl.project ORDER BY SUM(fl.line_count) DESC")
for r in c.fetchall():
    print(f"  {r[0]}: {r[1]:,}")

# Show all project files (non-3party)
print("\n=== PROJECT FILES (source) ===")
c.execute("SELECT path, description FROM files WHERE project = 'source' ORDER BY path")
for r in c.fetchall():
    print(f"  {r[0]}")

# Also show any files from other projects that look like our code
print("\n=== PROJECT FILES (other categories) ===")
c.execute("SELECT project, path FROM files WHERE project NOT IN ('source', '3party', 'Tools') AND project NOT LIKE '%3party%' ORDER BY project, path")
for r in c.fetchall():
    print(f"  [{r[0]}] {r[1]}")

# Show all crash risk files
print("\n=== ALL CRASH RISK FILES ===")
c.execute("SELECT file, risk_type, severity FROM crash_risks ORDER BY file")
for r in c.fetchall():
    if '3party' not in r[0] and 'easylogging' not in r[0]:
        print(f"  [{r[2]}] {r[0]}: {r[1]}")
    else:
        print(f"  [3RD] {r[0]}: {r[1]} ({r[2]})")

conn.close()
