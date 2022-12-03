from inc_noesis import *



def registerNoesisTypes():


    handle = noesis.register("Diablo II: Resurrected sprite", ".sprite")
    noesis.setHandlerTypeCheck(handle, noepyCheckType)
    noesis.setHandlerLoadRGBA(handle, D2RLoadSprite)


    noesis.logPopup()
    return 1
    

def noepyCheckType(data):
    return 1
 
 
 
 
 
 
 
def D2RLoadSprite(data, texList):

	MipData = []
	imgData = [] 
	bs = NoeBitStream(data)
	


	
	bs.seek(0x6, NOESEEK_ABS)
	unk1 = bs.readUShort()
	print("unk1 " + str(unk1))
	
	bs.seek(0x8, NOESEEK_ABS)
	t_width = bs.readUInt()
	print("t_width " + str(t_width))	
	
	bs.seek(0xc, NOESEEK_ABS)
	t_height = bs.readUInt()
	print("t_height " + str(t_height))	

	bs.seek(0x10, NOESEEK_ABS)
	unk4 = bs.readUInt()
	print("unk4 " + str(unk4))		
	
	bs.seek(0x14, NOESEEK_ABS)
	unk5 = bs.readUInt()
	print("unk5 " + str(unk5))		
	
	bs.seek(0x20, NOESEEK_ABS)
	RawSize = bs.readUInt()
	print("RawSize " + str(RawSize))	
	
	
	bs.seek(0x24, NOESEEK_ABS)
	unk6 = bs.readUInt()
	print("unk6 " + str(unk6))

	bs.seek(0x24, NOESEEK_ABS)
	RawData = bs.readBytes(RawSize)
	ddsData = rapi.imageDecodeRaw(RawData, t_width, t_height, "r8g8b8a8")
	
	
	texFmt = noesis.NOESISTEX_RGBA32
	tex1 = (NoeTexture(rapi.getInputName(), t_width, t_height, ddsData, texFmt))	
	texList.append(tex1)
	print("")
	print("")	
	return 1		
	
	
	
