options(direct=true,readsize=5000000)
load data
infile 'blobsource.dat'
   "str '<EORD>'"
truncate
into table BLOBDEST
fields terminated by '<EOFD>'
trailing nullcols
(
	ID,
	C1 CHAR(5000000) 
)
