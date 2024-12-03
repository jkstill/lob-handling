OPTIONS (DIRECT=TRUE)
load data
infile 'blobdest.dat'
   "str '<EORD>'"
into table BLOBDEST
fields terminated by '<EOFD>'
(
	ID,
	NAME,
	C1 CHAR(20000000)
)
