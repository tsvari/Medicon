import sqlite3, os

db_path = os.path.join('ToolsForAI', 'source_graph.db')
if not os.path.exists(db_path):
    print("DB not found")
    exit(1)

conn = sqlite3.connect(db_path)
c = conn.cursor()

def count_table(table, condition_col='file'):
    c.execute(f"SELECT COUNT(*) FROM {table}")
    total = c.fetchone()[0]
    c.execute(f"SELECT COUNT(*) FROM {table} WHERE {condition_col} LIKE '%3party%' OR {condition_col} LIKE '%easylogging%'")
    third = c.fetchone()[0]
    return total, total - third, third

# Crash risks
print("=== CRASH RISKS ===")
c.execute("SELECT COUNT(*) FROM crash_risks")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM crash_risks WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# Leak risks
print("\n=== LEAK RISKS ===")
c.execute("SELECT COUNT(*) FROM leak_risks")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM leak_risks WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# Raw pointers
print("\n=== RAW POINTER MEMBERS ===")
c.execute("SELECT COUNT(*) FROM raw_pointers")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM raw_pointers WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# C-style casts
print("\n=== C-STYLE CASTS ===")
c.execute("SELECT COUNT(*) FROM unsafe_casts")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM unsafe_casts WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# Dead methods
print("\n=== DEAD METHODS ===")
c.execute("SELECT COUNT(*) FROM dead_methods")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM dead_methods WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# Duplicate blocks
print("\n=== DUPLICATE BLOCKS ===")
c.execute("SELECT COUNT(*) FROM duplicate_blocks")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM duplicate_blocks WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# TODOs
print("\n=== TODOs ===")
c.execute("SELECT COUNT(*) FROM todos")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM todos WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# Review queue
print("\n=== REVIEW QUEUE ===")
c.execute("SELECT COUNT(*) FROM review_queue")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM review_queue WHERE file LIKE '%3party%'")
third = c.fetchone()[0]
print(f"  Total: {total}, Project: {total-third}, 3rd-party: {third}")

# File counts
print("\n=== FILE COUNTS ===")
c.execute("SELECT COUNT(*) FROM files")
total = c.fetchone()[0]
c.execute("SELECT COUNT(*) FROM files WHERE path LIKE '%3party%' OR path LIKE '%easylogging%'")
third = c.fetchone()[0]
print(f"  Total files: {total}, Project: {total-third}, 3rd-party: {third}")

# File lines
print("\n=== FILE LINES ===")
c.execute("SELECT SUM(lines) FROM file_lines")
total = c.fetchone()[0] or 0
c.execute("SELECT SUM(lines) FROM file_lines WHERE file LIKE '%3party%' OR file LIKE '%easylogging%'")
third = c.fetchone()[0] or 0
print(f"  Total lines: {total}, Project: {total-third}, 3rd-party: {third}")

# Project-specific crash risks
print("\n=== PROJECT CRASH RISK DETAILS ===")
c.execute("SELECT file, risk_type, severity FROM crash_risks WHERE file NOT LIKE '%3party%' AND file NOT LIKE '%easylogging%'")
for row in c.fetchall():
    print(f"  {row[0]}: {row[1]} ({row[2]})")

# Project-specific leak risks
print("\n=== PROJECT LEAK RISK DETAILS ===")
c.execute("SELECT file, risk_count FROM leak_risks WHERE file NOT LIKE '%3party%' AND file NOT LIKE '%easylogging%' ORDER BY risk_count DESC")
for row in c.fetchall()[:20]:
    print(f"  {row[0]}: {row[1]}")

# Project-specific review queue
print("\n=== PROJECT REVIEW QUEUE DETAILS ===")
c.execute("SELECT file, queue_type, priority_score, rationale FROM review_queue WHERE file NOT LIKE '%3party%'")
for row in c.fetchall():
    print(f"  {row[0]}: {row[1]} (score={row[2]}) - {row[3]}")

conn.close()
