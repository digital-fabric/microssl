#include "ruby.h"
#include "microssl.h"

ID ID_arity;
ID ID_backend_read;
ID ID_backend_recv;
ID ID_call;
ID ID_downcase;
ID ID_eof_p;
ID ID_eq;
ID ID_parser_read_method;
ID ID_read;
ID ID_readpartial;
ID ID_to_i;
ID ID_upcase;

static VALUE mPolyphony = Qnil;
static VALUE cError;

typedef struct parser {
  VALUE io;
  VALUE buffer;
  VALUE headers;
  int   current_request_rx;

  enum  read_method read_method;
  int   body_read_mode;
  int   body_left;
  int   request_completed;

  char *buf_ptr;
  int   buf_len;
  int   buf_pos;

} Parser_t;

VALUE cParser = Qnil;

static void Parser_mark(void *ptr) {
  Parser_t *parser = ptr;
  rb_gc_mark(parser->io);
  rb_gc_mark(parser->buffer);
  rb_gc_mark(parser->headers);
}

static void Parser_free(void *ptr) {
  xfree(ptr);
}

static size_t Parser_size(const void *ptr) {
  return sizeof(Parser_t);
}

static const rb_data_type_t Parser_type = {
  "Parser",
  {Parser_mark, Parser_free, Parser_size,},
  0, 0, 0
};

static VALUE Parser_allocate(VALUE klass) {
  Parser_t *parser;

  parser = ALLOC(Parser_t);
  return TypedData_Wrap_Struct(klass, &Parser_type, parser);
}

#define GetParser(obj, parser) \
  TypedData_Get_Struct((obj), Parser_t, &Parser_type, (parser))

static inline void get_polyphony() {
  if (mPolyphony != Qnil) return;

  mPolyphony = rb_const_get(rb_cObject, rb_intern("Polyphony"));
  rb_gc_register_mark_object(mPolyphony);
}

enum read_method detect_read_method(VALUE io) {
  if (rb_respond_to(io, ID_parser_read_method)) {
    VALUE method = rb_funcall(io, ID_parser_read_method, 0);
    if (method == SYM_stock_readpartial) return method_stock_readpartial;

    get_polyphony();
    if (method == SYM_backend_read)      return method_backend_read;
    if (method == SYM_backend_recv)      return method_backend_recv;

    return method_readpartial;
  }
  else if (rb_respond_to(io, ID_call)) {
    return method_call;
  }
  else
    rb_raise(rb_eRuntimeError, "Provided reader should be a callable or respond to #__parser_read_method__");
}

VALUE Parser_initialize(VALUE self, VALUE io) {
  Parser_t *parser;
  GetParser(self, parser);

  parser->io = io;
  parser->buffer = rb_str_new_literal("");
  parser->headers = Qnil;

  // pre-allocate the buffer
  rb_str_modify_expand(parser->buffer, INITIAL_BUFFER_SIZE);

  parser->read_method = detect_read_method(io);
  parser->body_read_mode = BODY_READ_MODE_UNKNOWN;
  parser->body_left = 0;

  parser->buf_ptr = 0;
  parser->buf_len = 0;
  parser->buf_pos = 0;

  return self;
}

////////////////////////////////////////////////////////////////////////////////

void Init_MicroSSL() {
  VALUE mMicroSSL;
  VALUE cParser;

  mMicroSSL = rb_define_module("MicroSSL");
  rb_gc_register_mark_object(mMicroSSL);
  cParser = rb_define_class_under(mMicroSSL, "Parser", rb_cObject);
  rb_define_alloc_func(cParser, Parser_allocate);

  cError = rb_define_class_under(mMicroSSL, "Error", rb_eRuntimeError);
  rb_gc_register_mark_object(cError);

  // backend methods
  rb_define_method(cParser, "initialize", Parser_initialize, 1);
  rb_define_method(cParser, "parse_headers", Parser_parse_headers, 0);
  rb_define_method(cParser, "read_body", Parser_read_body, 0);
  rb_define_method(cParser, "read_body_chunk", Parser_read_body_chunk, 1);
  rb_define_method(cParser, "complete?", Parser_complete_p, 0);

  ID_arity                  = rb_intern("arity");
  ID_backend_read           = rb_intern("backend_read");
  ID_backend_recv           = rb_intern("backend_recv");
  ID_call                   = rb_intern("call");
  ID_downcase               = rb_intern("downcase");
  ID_eof_p                  = rb_intern("eof?");
  ID_eq                     = rb_intern("==");
  ID_parser_read_method     = rb_intern("__parser_read_method__");
  ID_read                   = rb_intern("read");
  ID_readpartial            = rb_intern("readpartial");
  ID_to_i                   = rb_intern("to_i");
  ID_upcase                 = rb_intern("upcase");

  NUM_max_headers_read_length = INT2NUM(MAX_HEADERS_READ_LENGTH);
  NUM_buffer_start = INT2NUM(0);
  NUM_buffer_end = INT2NUM(-1);

  GLOBAL_STR(STR_pseudo_method,       ":method");
  GLOBAL_STR(STR_pseudo_path,         ":path");
  GLOBAL_STR(STR_pseudo_protocol,     ":protocol");
  GLOBAL_STR(STR_pseudo_rx,           ":rx");

  GLOBAL_STR(STR_chunked,             "chunked");
  GLOBAL_STR(STR_content_length,      "content-length");
  GLOBAL_STR(STR_transfer_encoding,   "transfer-encoding");

  SYM_backend_read = ID2SYM(ID_backend_read);
  SYM_backend_recv = ID2SYM(ID_backend_recv);
  SYM_stock_readpartial = ID2SYM(rb_intern("stock_readpartial"));

  rb_global_variable(&mMicroSSL);
}

void Init_h1p_ext() {
  Init_MicroSSL();
}
