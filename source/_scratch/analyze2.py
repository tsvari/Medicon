import sqlite3, os

db_path = os.path.join('ToolsForAI', 'source_graph.db')
conn = sqlite3.connect(db_path)
c = conn.cursor()

def count_with_file(table, file_col):
    c.execute(f"SELECT COUNT(*) FROM {table}")
    total = c.fetchone()[0]
    c.execute(f"SELECT COUNT(*) FROM {table} WHERE {file_col} LIKE '%3party%' OR {file_col} LIKE '%easylogging%'")
    third = c.fetchone()[0]
    proj = total - third
    pct = (proj / total * 100) if total > 0 else 0
    print(f"  {table}: Total={total}, Project={proj} ({pct:.1f}%), 3rd-party={third}")

print("=== ISSUE DISTRIBUTION (Project vs 3rd-party) ===")
count_with_file('crash_risks', 'file')
count_with_file('leak_risks', 'file')
count_with_file('raw_pointers', 'file')
count_with_file('unsafe_casts', 'file')
count_with_file('todos', 'file')

# dead_methods uses header_file
c.execute("SELECT COUNT(*) FROM dead_methods")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM dead_methods WHERE header_file LIKE '%3party%' OR header_file LIKE '%easylogging%'")
third = c.fetchone()[0]
proj = total - third
print(f"  dead_methods: Total={total}, Project={proj} ({proj/total*100:.1f}%), 3rd-party={third}")

# duplicate_blocks
c.execute("SELECT COUNT(*) FROM duplicate_blocks")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM duplicate_blocks WHERE file_a LIKE '%3party%' OR file_a LIKE '%easylogging%'")
third = c.fetchone()[0]
proj = total - third
print(f"  duplicate_blocks: Total={total}, Project={proj} ({proj/total*100:.1f}%), 3rd-party={third}")

# file counts
c.execute("SELECT COUNT(*) FROM files")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM files WHERE path LIKE '%3party%' OR path LIKE '%easylogging%'")
third = c.fetchone()[0]
proj = total - third
print(f"\n=== FILE COUNTS ===")
print(f"  Total files: {total}, Project: {proj} ({proj/total*100:.1f}%), 3rd-party: {third}")

# lines
c.execute("SELECT SUM(line_count) FROM file_lines")
total = c.fetchone()[0] or 0
c.execute("SELECT SUM(line_count) FROM file_lines WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0] or 0
proj = total - third
print(f"\n=== LINE COUNTS ===")
print(f"  Total lines: {total:,}, Project: {proj:,} ({proj/total*100:.1f}%), 3rd-party: {third:,}")

# Review queue
c.execute("SELECT COUNT(*) FROM review_queue")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM review_queue WHERE file LIKE '%3party%'")
third = c.fetchone()[0]
proj = total - third
print(f"\n=== REVIEW QUEUE ===")
print(f"  Total: {total}, Project: {proj}, 3rd-party: {third}")

conn.close()
