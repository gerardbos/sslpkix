#pragma once

//#include <cassert>
#include <iostream>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include "sslpkix/x509/cert.h"
#include "sslpkix/common.h"
#include "sslpkix/non_copyable.h"
#if 0
#include "sslpkix/openssl/apps/verify.h"
#endif

namespace sslpkix {

class CertificateStore : non_copyable  {
	// More info: http://www.umich.edu/~x509/ssleay/x509_store.html
public:
	typedef X509_STORE handle_type;
public:
	CertificateStore()
		: _handle(NULL)
	{
	}
	~CertificateStore() {
		release();
	}
	handle_type *handle() const {
		//assert(_handle != NULL);
		return _handle;
	}
	bool create() {
		release();
		_handle = X509_STORE_new();
		if (_handle == NULL) {
			std::cerr << "Failed to create certificate store" << std::endl;
			return false;
		}
		// The callback is only needed for more descriptive error messages, etc
		#if 0
		X509_STORE_set_verify_cb_func(_store, verify_callback);
		#endif
		return true;
	}
	void set_flags(long flags) {
		X509_STORE_set_flags(_handle, flags);
	}
	/*
	// More info: http://www.openssl.org/docs/crypto/X509_VERIFY_PARAM_set_flags.html
	bool set_param() {
		X509_VERIFY_PARAM *vpm = NULL;
		if (vpm)
			X509_STORE_set1_param(_handle, vpm);
		if (vpm)
			X509_VERIFY_PARAM_free(vpm);
	}
	*/
	bool add_trusted_cert(Certificate& cert) {
		int ret = X509_STORE_add_cert(_handle, cert.handle());
		return ret != 0;
	}
protected:
	void release() {
		if (_handle != NULL) {
			X509_STORE_free(_handle);
			_handle = NULL;
		}
	}
protected:
	handle_type *_handle;
};

class CertificateStoreContext : non_copyable  {
public:
	typedef X509_STORE_CTX handle_type;
public:
	CertificateStoreContext() : _handle(NULL) {
	}
	~CertificateStoreContext() {
		release();
	}
	handle_type *handle() const {
		//assert(_handle != NULL);
		return _handle;
	}
	bool create() {
		release();
		_handle = X509_STORE_CTX_new();
		if (_handle == NULL) {
			std::cerr << "Failed to create certificate store context" << std::endl;
			return false;
		}
		return true;
	}
protected:
	void release() {
		if (_handle != NULL) {
			X509_STORE_CTX_free(_handle);
			_handle = NULL;
		}
	}
protected:
	handle_type *_handle;
};

class CertificateVerifier {
public:
	// More info: http://www.openssl.org/docs/apps/verify.html#VERIFY_OPERATION
	bool verify(
		CertificateStore& store,
		CertificateStoreContext& ctx,
		Certificate& cert,
		unsigned long flags UNUSED,
		int purpose)
	{
		CertificateStoreContext::handle_type *pctx = ctx.handle();
		int ret = X509_STORE_CTX_init(pctx, store.handle(), NULL, NULL);
		if (!ret) {
			std::cerr << "Failed to initialize certificate store context" << std::endl;
			return false;
		}
		if (purpose >= 0)
			X509_STORE_CTX_set_purpose(pctx, purpose);
		// Set the certificate to be checked
		X509_STORE_CTX_set_cert(pctx, cert.handle());
		// Check the certificate
		ret = X509_verify_cert(pctx);
		// Don't free, just cleanup for reuse
		X509_STORE_CTX_cleanup(pctx);
		if (ret < 0) {
			std::cerr << "Error: " << X509_STORE_CTX_get_error(pctx) << std::endl;
		}
		return ret == 1;
	}
};

} // namespace sslpkix
