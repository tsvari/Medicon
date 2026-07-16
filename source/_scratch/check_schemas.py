import sqlite3, os

db_path = os.path.join('ToolsForAI', 'source_graph.db')
conn = sqlite3.connect(db_path)
c = conn.cursor()

# Check schemas of key tables
for tbl in ['dead_methods', 'duplicate_blocks', 'todos', 'review_queue', 'crash_risks', 'leak_risks', 'raw_pointers', 'unsafe_casts', 'file_lines', 'files']:
    c.execute(f"PRAGMA table_info({tbl})")
    cols = c.fetchall()
    print(f"\n=== {tbl} ===")
    for col in cols:
        print(f"  {col[1]} ({col[2]})")

conn.close()
