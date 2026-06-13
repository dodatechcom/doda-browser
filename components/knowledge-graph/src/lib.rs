use rusqlite::Connection;
use serde::{Deserialize, Serialize};
use url::Url;

#[derive(Debug, Serialize, Deserialize)]
pub struct Page {
    pub url: String,
    pub title: String,
    pub text_content: String,
    pub visited_at: String,
    pub tags: Vec<String>,
}

pub struct KnowledgeGraph {
    conn: Connection,
}

impl KnowledgeGraph {
    pub fn new(path: &str) -> Self {
        let conn = Connection::open(path).unwrap();
        conn.execute_batch(
            "CREATE TABLE IF NOT EXISTS pages (
                url TEXT PRIMARY KEY,
                title TEXT NOT NULL,
                text_content TEXT NOT NULL,
                visited_at TEXT NOT NULL,
                domain TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS tags (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                url TEXT NOT NULL,
                tag TEXT NOT NULL,
                FOREIGN KEY (url) REFERENCES pages(url)
            );
            CREATE TABLE IF NOT EXISTS links (
                source_url TEXT NOT NULL,
                target_url TEXT NOT NULL,
                created_at TEXT NOT NULL,
                PRIMARY KEY (source_url, target_url)
            );
            CREATE VIRTUAL TABLE IF NOT EXISTS pages_fts USING fts5(
                title, text_content, content=pages, content_rowid=rowid
            );"
        ).unwrap();
        Self { conn }
    }

    pub fn index_page(&self, page: &Page) {
        let domain = Url::parse(&page.url)
            .map(|u| u.host_str().unwrap_or("").to_string())
            .unwrap_or_default();

        self.conn.execute(
            "INSERT OR REPLACE INTO pages (url, title, text_content, visited_at, domain)
             VALUES (?1, ?2, ?3, ?4, ?5)",
            rusqlite::params![page.url, page.title, page.text_content, page.visited_at, domain],
        ).unwrap();
    }

    pub fn search(&self, query: &str) -> Vec<Page> {
        let mut stmt = self.conn.prepare(
            "SELECT p.url, p.title, p.text_content, p.visited_at
             FROM pages_fts f
             JOIN pages p ON p.rowid = f.rowid
             WHERE pages_fts MATCH ?1
             LIMIT 20"
        ).unwrap();

        stmt.query_map(rusqlite::params![query], |row| {
            Ok(Page {
                url: row.get(0)?,
                title: row.get(1)?,
                text_content: row.get(2)?,
                visited_at: row.get(3)?,
                tags: Vec::new(),
            })
        }).unwrap().filter_map(|r| r.ok()).collect()
    }
}
