template ".checkfile files"

description "SV Foster's FileHash .checkfile structure"

applies_to file
requires 0 "5356464F53544552"
requires 8 "48415348"

begin
	section "File Header"
		read-only char[8]	"Author Magic"
		read-only char[4]	"File Header Magic"
		uint32 "Header Size, bytes"
		hex 2 "Platform (HEX)"
		hex 2 "Algorithms (HEX)"
		move -2
		binary[1] "Algorithms (BIN)"
		move -1
		uint_flex "0" "MD2"
		move -4
		uint_flex "1" "MD4"
		move -4
		uint_flex "2" "MD5"			
		move -4
		uint_flex "3" "SHA1"			
		move -4
		uint_flex "4" "SHA256"			
		move -4
		uint_flex "5" "SHA384"
		move -4
		uint_flex "6" "SHA512"
		move -4
		move 2
		uint32 "Files Total Count"
		hex 8 "Files Total Size (HEX)"
		move -8
		uint32 "Files Total Size, High"
		uint32 "Files Total Size, Low"
	endsection

	numbering 1 {
		Section "File Entry ~"
			uint32 "Record Size, bytes"
			hex 8 "Files Size (HEX)"
			move -8
			uint32 "Files Size, High"
			uint32 "Files Size, Low"
			IfEqual MD2 1
				hex 16 "MD2 hash"
			Endif
			IfEqual MD4 1
				hex 16 "MD4 hash"
			Endif
			IfEqual MD5 1			
				hex 16 "MD5 hash"
			Endif
			IfEqual SHA1 1			
				hex 20 "SHA1 hash"
			Endif
			IfEqual SHA256 1			
				hex 32 "SHA256 hash"
			Endif
			IfEqual SHA384 1			
				hex 48 "SHA384 hash"
			Endif
			IfEqual SHA512 1			
				hex 64 "SHA512 hash"
			Endif		
			
			zstring16 "File Name"
		endSection	
		}["Files Total Count"]
end
