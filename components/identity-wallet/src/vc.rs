use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct VerifiableCredential {
    pub context: Vec<String>,
    pub id: String,
    pub types: Vec<String>,
    pub issuer: String,
    pub issuance_date: String,
    pub expiration_date: Option<String>,
    pub credential_subject: serde_json::Value,
    pub proof: Option<Proof>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Proof {
    pub type_: String,
    pub created: String,
    pub verification_method: String,
    pub proof_purpose: String,
    pub proof_value: String,
}

impl VerifiableCredential {
    pub fn new(
        issuer: String,
        subject: serde_json::Value,
        subject_id: String,
    ) -> Self {
        Self {
            context: vec![
                "https://www.w3.org/2018/credentials/v1".into(),
            ],
            id: format!("urn:uuid:{}", uuid::Uuid::new_v4()),
            types: vec!["VerifiableCredential".into()],
            issuer,
            issuance_date: chrono_now(),
            expiration_date: None,
            credential_subject: serde_json::json!({
                "id": subject_id,
                "claims": subject,
            }),
            proof: None,
        }
    }
}

fn chrono_now() -> String {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| format!("{}", d.as_secs()))
        .unwrap_or_default()
}
