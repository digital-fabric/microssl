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

void Init_MicroSSL() {
  VALUE mMicroSSL;
  VALUE cContext;

  mMicroSSL = rb_define_module("MicroSSL");
  rb_gc_register_mark_object(mMicroSSL);
  cContext = rb_define_class_under(mMicroSSL, "Context", rb_cObject);
  rb_define_alloc_func(cContext, Context_allocate);

  // cError = rb_define_class_under(mMicroSSL, "Error", rb_eRuntimeError);
  // rb_gc_register_mark_object(cError);

  // backend methods
  rb_define_method(cContext, "initialize", Context_initialize, 0);

  rb_define_method(cContext, "security_level", Context_security_level_get, 0);
  rb_define_method(cContext, "security_level=", Context_security_level_set, 1);

}

void Init_microssl_ext() {
  Init_MicroSSL();
}
