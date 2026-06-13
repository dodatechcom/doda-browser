use crate::did::DID;
use crate::vc::VerifiableCredential;
use crate::storage::Storage;

pub struct Wallet {
    pub dids: Vec<DID>,
    pub credentials: Vec<VerifiableCredential>,
    storage: Storage,
}

impl Wallet {
    pub fn new(path: &str) -> Self {
        Self {
            dids: Vec::new(),
            credentials: Vec::new(),
            storage: Storage::new(path),
        }
    }

    pub fn create_identity(&mut self) -> &DID {
        let did = DID::generate();
        self.dids.push(did);
        self.dids.last().unwrap()
    }

    pub fn sign_in_with_did(&self, domain: &str, did: &DID) -> Vec<u8> {
        let payload = format!("{}:{}", domain, did.did);
        did.sign(payload.as_bytes())
    }

    pub fn store_credential(&mut self, vc: VerifiableCredential) {
        self.storage.save_credential(&vc);
        self.credentials.push(vc);
    }

    pub fn list_credentials(&self) -> &[VerifiableCredential] {
        &self.credentials
    }
}
