#include "ruby.h"
#include "microssl.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/x509.h>

#ifndef SSL_OP_NO_COMPRESSION
#define SSL_OP_NO_COMPRESSION 0
#endif

typedef struct context {
  SSL_CTX *ctx;
} Context_t;

static void Context_mark(void *ptr) {
  // Context_t *context = ptr;
}

static void Context_free(void *ptr) {
  Context_t *context = ptr;
  SSL_CTX_free(context->ctx);
  xfree(ptr);
}

static size_t Context_size(const void *ptr) {
  return sizeof(Context_t);
}

static const rb_data_type_t Context_type = {
  "Context",
  {Context_mark, Context_free, Context_size,},
  0, 0, 0
};

static VALUE Context_allocate(VALUE klass) {
  Context_t *context;

  context = ALLOC(Context_t);
  return TypedData_Wrap_Struct(klass, &Context_type, context);
}

#define GetContext(obj, context) \
  TypedData_Get_Struct((obj), Context_t, &Context_type, (context))

////////////////////////////////////////////////////////////////////////////////

VALUE Context_initialize(VALUE self) {
  Context_t *context;
  GetContext(self, context);

  #ifdef HAVE_DTLS_METHOD
    context->ctx = SSL_CTX_new(DTLS_method());
  #else
    context->ctx = SSL_CTX_new(DTLSv1_method());
  #endif

  return self;
}

VALUE Context_security_level_get(VALUE self) {
  Context_t *context;
  GetContext(self, context);

  #if defined(HAVE_SSL_CTX_GET_SECURITY_LEVEL)
    return INT2NUM(SSL_CTX_get_security_level(context->ctx));
  #else
    return INT2FIX(0);
  #endif
}

VALUE Context_security_level_set(VALUE self, VALUE level) {
  Context_t *context;
  rb_check_frozen(self);
  GetContext(self, context);

  #if defined(HAVE_SSL_CTX_GET_SECURITY_LEVEL)
    SSL_CTX_set_security_level(context->ctx, NUM2INT(level));
  #else
    if (NUM2INT(level) != 0)
      rb_raise(
        rb_eNotImpError,
        "setting security level to other than 0 is not supported in this version of OpenSSL"
      );
  #endif

  return level;
}

BIO *str2bio(VALUE obj)
{
  BIO *bio;

  StringValue(obj);
  bio = BIO_new_mem_buf(RSTRING_PTR(obj), RSTRING_LENINT(obj));
  if (!bio) rb_raise(rb_eRuntimeError, "BIO_new_mem_buf");

  return bio;
}

X509 *string_to_x509_cert(VALUE string) {
  BIO *in = str2bio(string);
  X509 *x509 = d2i_X509_bio(in, NULL);
  if (!x509) {
    BIO_reset(in);
    x509 = PEM_read_bio_X509(in, NULL, NULL, NULL);
  }
  BIO_free(in);
  if (!x509) rb_raise(rb_eRuntimeError, "string_to_x509_cert");

  return x509;
}

EVP_PKEY *string_to_rsa_pkey(VALUE string) {
  EVP_PKEY *pkey;
  RSA *rsa;
  BIO *in;
  // VALUE arg, pass;

  pkey = EVP_PKEY_new();

  // pass = ossl_pem_passwd_value(pass);
  // arg = ossl_to_der_if_possible(arg);
  in = str2bio(string);
  rsa = PEM_read_bio_RSAPrivateKey(in, NULL, NULL, NULL);
  if (!rsa) {
    BIO_reset(in);
    rsa = PEM_read_bio_RSA_PUBKEY(in, NULL, NULL, NULL);
  }
  if (!rsa) {
    BIO_reset(in);
    rsa = d2i_RSAPrivateKey_bio(in, NULL);
  }
  if (!rsa) {
    BIO_reset(in);
    rsa = d2i_RSA_PUBKEY_bio(in, NULL);
  }
  if (!rsa) {
    BIO_reset(in);
    rsa = PEM_read_bio_RSAPublicKey(in, NULL, NULL, NULL);
  }
  if (!rsa) {
    BIO_reset(in);
    rsa = d2i_RSAPublicKey_bio(in, NULL);
  }
  BIO_free(in);
  if (!rsa) rb_raise(rb_eRuntimeError, "Neither PUB key nor PRIV key");
  if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
    RSA_free(rsa);
    rb_raise(rb_eRuntimeError, "Failed to assign RSA");
  }

  return pkey;
}


VALUE Context_add_certificate(VALUE self, VALUE private_key, VALUE certificate, VALUE chain) {
  // Context_t *context;
  // rb_check_frozen(self);
  // GetContext(self, context);

  return self;
}

void Init_MicroSSL() {
  VALUE mMicroSSL;
  VALUE cContext;

  mMicroSSL = rb_define_module("MicroSSL");
  rb_gc_register_mark_object(mMicroSSL);
  cContext = rb_define_class_under(mMicroSSL, "Context", rb_cObject);
  rb_define_alloc_func(cContext, Context_allocate);

  /* Version of OpenSSL that Puma was compiled with */
  rb_define_const(mMicroSSL, "OPENSSL_VERSION", rb_str_new2(OPENSSL_VERSION_TEXT));

  #if !defined(LIBRESSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER >= 0x10100000
    /* Version of OpenSSL that Puma loaded with */
    rb_define_const(mMicroSSL, "OPENSSL_LIBRARY_VERSION", rb_str_new2(OpenSSL_version(OPENSSL_VERSION)));
  #else
    rb_define_const(mMicroSSL, "OPENSSL_LIBRARY_VERSION", rb_str_new2(SSLeay_version(SSLEAY_VERSION)));
  #endif

  #if defined(OPENSSL_NO_SSL3) || defined(OPENSSL_NO_SSL3_METHOD)
    /* True if SSL3 is not available */
    rb_define_const(mMicroSSL, "OPENSSL_NO_SSL3", Qtrue);
  #else
    rb_define_const(mMicroSSL, "OPENSSL_NO_SSL3", Qfalse);
  #endif

  #if defined(OPENSSL_NO_TLS1) || defined(OPENSSL_NO_TLS1_METHOD)
    /* True if TLS1 is not available */
    rb_define_const(mMicroSSL, "OPENSSL_NO_TLS1", Qtrue);
  #else
    rb_define_const(mMicroSSL, "OPENSSL_NO_TLS1", Qfalse);
  #endif

  #if defined(OPENSSL_NO_TLS1_1) || defined(OPENSSL_NO_TLS1_1_METHOD)
    /* True if TLS1_1 is not available */
    rb_define_const(mMicroSSL, "OPENSSL_NO_TLS1_1", Qtrue);
  #else
    rb_define_const(mMicroSSL, "OPENSSL_NO_TLS1_1", Qfalse);
  #endif



  // cError = rb_define_class_under(mMicroSSL, "Error", rb_eRuntimeError);
  // rb_gc_register_mark_object(cError);

  // backend methods
  rb_define_method(cContext, "initialize", Context_initialize, 0);

  rb_define_method(cContext, "security_level", Context_security_level_get, 0);
  rb_define_method(cContext, "security_level=", Context_security_level_set, 1);

  rb_define_method(cContext, "add_certificate", Context_add_certificate, 3);
}

void Init_microssl_ext() {
  Init_MicroSSL();
}
