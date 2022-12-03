#Import/Export script for Diablo II: Resurrected .texture format
#by Jayn23 at Xentax fourm


from inc_noesis import *
import math

TEX_TYPE = 'Default'
#TEX_TYPE = 'DXT1'
#TEX_TYPE = 'DXT5'
#TEX_TYPE = 'BC4'
#TEX_TYPE = 'RGBA32'
SWIZZLE = False

def registerNoesisTypes():
    handle = noesis.register("Diablo II: Resurrected texture", ".texture")
    noesis.setHandlerTypeCheck(handle, noepyCheckType)
    noesis.setHandlerLoadRGBA(handle, noepyLoadRGBA)
    noesis.setHandlerWriteRGBA(handle, textureWriteRGBA)
    noesis.addOption(handle, '-DXT1', "Set Ootput Format DXT1", 0)
    noesis.addOption(handle, '-DXT5', "Set Ootput Format DXT5", 0)
    noesis.addOption(handle, '-BC4', "Set Ootput Format BC4", 0)
    noesis.addOption(handle, '-RGBA32', "Set Ootput Format RGBA32", 0)
    noesis.addOption(handle, '-SWIZZLE', "Swap red and green channels for normal maps", 0)
    noesis.logPopup()
    return 1
    

def noepyCheckType(data):
    '''Verify that the format is supported by this plugin. Default yes'''
    return 1
 
 
class MipInfo :
      
      def __init__(self):
            self.start = 0
            self.size = 0

def noepyLoadRGBA(data, texList):
        
        MipData = []
        imgData = []

        f = NoeBitStream(data, NOE_LITTLEENDIAN)
        MagicValue = f.readUInt()
        Type = f.readUShort()
        print(Type)
        unk = f.readUShort()
        print(unk)
        imgWidth = f.readUInt()
        print(imgWidth)
        imgHeight = f.readUInt()
        print(imgHeight)
        depth = f.readUInt()
        #layoutType and layoutData
        f.seek(8, 1)
        MipLevels = f.readUInt()
        print(MipLevels)
        Channel_Count = f.readUInt()


        for i in range(MipLevels):
            Temp = MipInfo ()
            Temp.size = f.readUInt()
            print(Temp.size)
            Current = f.tell() 
            Temp.start = f.readUInt()
            Temp.start += Current
            print(Temp.start)
            MipData.append(Temp)

        for Map in MipData:
            f.seek(Map.start,0)
            imgData += f.readBytes(Map.size)

        data = bytearray(imgData)

        if Type == 31: #RGBA32
             texFmt = noesis.NOESISTEX_RGBA32
        elif Type == 57 or Type == 58: # DXT1  
             texFmt = noesis.NOESISTEX_DXT1
        elif Type == 61 or Type == 62: # DXT5   
            texFmt = noesis.NOESISTEX_DXT5
        elif Type == 63: # BC4
            #data = rapi.imageDecodeDXT(data, imgWidth, imgHeight, noesis.FOURCC_ATI1)
            data = rapi.imageDecodeDXT(data, imgWidth, imgHeight, noesis.FOURCC_BC4)
            texFmt = noesis.NOESISTEX_RGBA32

        tex = NoeTexture(rapi.getInputName(), imgWidth, imgHeight, data, texFmt)
        texList.append(tex)

        return 1


            
            
def textureWriteRGBA(data, width, height, bs):
    
    MAGIC = 0x2845443C
    MipLevels = 0
    mipWidth = width
    mipHeight = height
    TextureData = []
    AllData = bytearray()
    Sum = 0
    

    if TEX_TYPE == 'DXT1' or noesis.optWasInvoked('-DXT1'):
        type = 57
        texFmt = noesis.NOE_ENCODEDXT_BC1
    elif TEX_TYPE == 'DXT5' or noesis.optWasInvoked('-DXT5'):
        type = 61
        #texFmt = noesis.NOE_ENCODEDXT_BC3
        #texFmt = noesis.FOURCC_BC3
        texFmt = noesis.FOURCC_DXT5
    elif TEX_TYPE == 'BC4' or noesis.optWasInvoked('-BC4'):
        type = 63
        texFmt = noesis.NOE_ENCODEDXT_BC4
    elif TEX_TYPE == 'RGBA32' or noesis.optWasInvoked('-RGBA32'):
        type = 31
        texFmt = noesis.NOESISTEX_RGBA32
    else:
        print('Unsupported format')
        return 0

    #Normal map the swap green and red channels
    if  noesis.optWasInvoked('-SWIZZLE') or SWIZZLE:
        data = rapi.imageNormalSwizzle(data, width, height, 1, 1, 0)

    if type == 31:
        imgData = rapi.imageEncodeRaw(data, width, height, "r8 g8 b8 a8")
        MipSize = len(imgData)
        TextureData.append(imgData)
        HeaderSize = 44
        
    elif type == 57 or type == 61 or type == 63:

        while mipWidth > 4 or mipHeight > 4:
            mipData = rapi.imageResample(data, width, height, mipWidth, mipHeight)
            imgData = rapi.imageEncodeDXT(mipData, 4, mipWidth, mipHeight, texFmt)
            TextureData.append(imgData)
            MipSize = len(imgData)
            
            if mipWidth > 4:
                mipWidth //= 2
                
            if mipHeight > 4:
                mipHeight //= 2
            
    HeaderSize = 36 + 8 *len(TextureData)
    bs.writeUInt(MAGIC)
    bs.writeUShort(type)
    MipUnk = 258 + (len(TextureData) - 1) * 256
    bs.writeUShort(MipUnk)
    bs.writeUInt(width)
    bs.writeUInt(height)
    #depth
    bs.writeUInt(0x1)
    #layoutType and layoutData
    bs.writeUInt64(0x0)
    MipLevels = len(TextureData)
    bs.writeUInt(MipLevels)
    bs.writeUInt(0x4)
    
    for i in range(MipLevels):
        bs.writeUInt(len(TextureData[i]))
        Current = bs.tell()
        MipStartLocation = HeaderSize - Current + Sum
        Sum += len(TextureData[i])
        bs.writeUInt(MipStartLocation)
        AllData += TextureData[i]
      
    bs.writeBytes(AllData)
    
    return 1