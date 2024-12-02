
options(direct=true,readsize=10000000)
load data
infile 'blobsource.dat'
   "str '<EORD>'"
truncate
into table BLOBDEST
fields terminated by '<EOFD>'
trailing nullcols
(
	ID,
	NAME,
	C1 CHAR(10000000) 
)

