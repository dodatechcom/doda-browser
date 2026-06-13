use ed25519_dalek::{Keypair, Signature, Signer, Verifier};
use rand::rngs::OsRng;
use sha2::{Digest, Sha256};
use base64::{Engine as _, engine::general_purpose};

#[derive(Debug, Clone)]
pub struct DID {
    pub method: String,
    pub id_string: String,
    pub did: String,
    pub keypair: Keypair,
}

impl DID {
    pub fn generate() -> Self {
        let mut csprng = OsRng;
        let keypair: Keypair = Keypair::generate(&mut csprng);

        let pub_key_bytes = keypair.public.as_bytes();
        let hash = Sha256::digest(pub_key_bytes);
        let id_string = general_purpose::URL_SAFE_NO_PAD.encode(&hash[..16]);
        let did = format!("did:key:z{}", id_string);

        Self {
            method: "key".into(),
            id_string,
            did,
            keypair,
        }
    }

    pub fn sign(&self, message: &[u8]) -> Vec<u8> {
        self.keypair.sign(message).to_bytes().to_vec()
    }

    pub fn verify(&self, message: &[u8], signature: &[u8]) -> bool {
        let sig = Signature::from_slice(signature).unwrap();
        self.keypair.verify(message, &sig).is_ok()
    }
}
