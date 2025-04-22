
CREATE TYPE aq$_jms_text_message FORCE AS OBJECT (
  header_properties SYS.AQ$_JMS_HEADER,
  text_vc           VARCHAR2(4000)
)
/


