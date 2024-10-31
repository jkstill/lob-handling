
create or replace
function clob2blob (p_clob clob) return blob deterministic as
  l_tgt_idx  int := 1;
  l_src_idx  int := 1;
  l_blob     blob;
  l_lang     int := dbms_lob.default_lang_ctx;
  l_err      int := dbms_lob.warn_inconvertible_char;
begin

  dbms_lob.createtemporary(
    lob_loc => l_blob,
    cache   => true);

  dbms_lob.converttoblob(
   dest_lob    =>l_blob,
   src_clob    =>p_clob,
   amount      =>dbms_lob.lobmaxsize,
   dest_offset =>l_tgt_idx,
   src_offset  =>l_src_idx,
   blob_csid   =>dbms_lob.default_csid,
   lang_context=>l_lang,
   warning     =>l_err);

   return l_blob;
end;
/

