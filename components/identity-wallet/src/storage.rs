use crate::vc::VerifiableCredential;
use rusqlite::Connection;

pub struct Storage {
    conn: Connection,
}

impl Storage {
    pub fn new(path: &str) -> Self {
        let conn = Connection::open(path).unwrap();
        conn.execute_batch(
            "CREATE TABLE IF NOT EXISTS credentials (
                id TEXT PRIMARY KEY,
                issuer TEXT NOT NULL,
                subject TEXT NOT NULL,
                issued_at TEXT NOT NULL,
                data TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS identities (
                did TEXT PRIMARY KEY,
                public_key BLOB NOT NULL,
                secret_key BLOB NOT NULL,
                created_at TEXT NOT NULL
            );"
        ).unwrap();
        Self { conn }
    }

    pub fn save_credential(&self, vc: &VerifiableCredential) {
        self.conn.execute(
            "INSERT OR REPLACE INTO credentials (id, issuer, subject, issued_at, data) VALUES (?1, ?2, ?3, ?4, ?5)",
            rusqlite::params![
                vc.id,
                vc.issuer,
                vc.credential_subject.to_string(),
                vc.issuance_date,
                serde_json::to_string(vc).unwrap()
            ],
        ).unwrap();
    }
}
