import sqlite3, os

db_path = os.path.join('ToolsForAI', 'source_graph.db')
if os.path.exists(db_path):
    conn = sqlite3.connect(db_path)
    c = conn.cursor()
    c.execute("SELECT name FROM sqlite_master WHERE type='table'")
    tables = [t[0] for t in c.fetchall()]
    print("Tables:", tables)
    
    # Check leak_risk_files table
    if 'leak_risk_files' in tables:
        c.execute("SELECT file, risk_count FROM leak_risk_files ORDER BY risk_count DESC LIMIT 50")
        rows = c.fetchall()
        project_risks = [r for r in rows if '3party' not in r[0] and 'easylogging' not in r[0]]
        thirdparty_risks = [r for r in rows if '3party' in r[0] or 'easylogging' in r[0]]
        print(f"\nTotal leak risk files: {len(rows)}")
        print(f"Project code: {len(project_risks)}")
        print(f"3rd-party code: {len(thirdparty_risks)}")
        if project_risks:
            print("\nTop project leak risks:")
            for f, c in project_risks[:10]:
                print(f"  {f}: {c}")
    
    conn.close()
else:
    print("DB not found")
