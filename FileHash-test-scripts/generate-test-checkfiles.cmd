set exe="..\FileHash-exe-IA-32\filehash.exe"
set fdemo=..\FileHash-test-structures\

%exe% c /MD2 0001-plain-MD2.checkfile %fdemo%
%exe% c /MD4 0002-plain-MD4.checkfile %fdemo%
%exe% c /MD5 0003-plain-MD5.checkfile %fdemo%
%exe% c /SHA1 0004-plain-SHA1.checkfile %fdemo%
%exe% c /SHA256 0005-plain-SHA256.checkfile %fdemo%
%exe% c /SHA384 0006-plain-SHA384.checkfile %fdemo%
%exe% c /SHA512 0007-plain-SHA512.checkfile %fdemo%
%exe% c /AlgoALL 0008-plain-ALL.checkfile %fdemo%
%exe% c /AlgoALL /recursive 0009-recursive-ALL.checkfile %fdemo%
%exe% c /MD5 0010-wildcard-MD5.checkfile %fdemo%*.txt
%exe% c 0018-plain-SHA512.checkfile %fdemo%*.txt %fdemo%d.111
