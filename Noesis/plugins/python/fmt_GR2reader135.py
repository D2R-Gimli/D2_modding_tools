# Granny .GR2 Reader script by Jayn23 at XeNTaX fourm
# Credit to norbyte from DOS fourm - his source code for gr2 to collada was my main reference
# can be found at https://github.com/Norbyte/lslib
# AOE Online/AOEIII FD format based on source by kangcliff
# can be found https://github.com/kangcliff/Age-of-Empires-III


# Version 1.3.5
# Last updated 14.06.2021


from math import *
from inc_noesis import *
import struct
import ctypes
import io
import sys
import math 
import bisect
import re
import zlib

#================================================================
#Plugin Options
#================================================================

#General Granny Options
MULTIFILE = 0                       #when set to 1 all files in folder are loaded and mesh/skeleten are merged, 2 loads all meshes in folder, 3 loads all skeleton.
SMART_DETECTION = 0                 #when comnining meshes (MULTIFILE)  1 - check for duplicate bone names and delete, 2 - by comparing InverseWorld transform and parent bone name in addition to bone name
ANIMATION_TRACK = 0                 #for gr2  with multiple Tracks, choose Track to load, 1 - will load first animation, 2 second animation etc..
ANIMATION_MODE = 0                  #switch between Animation modes, 1 - load paired animation file, 2 - load animation from main file, 0 - disable animation loading
SKELETON_LOAD = 0                   #Enables loading a paired skeleton file (1 = on, 0 = off)
MERGE_SCENE = 0                     #if set  = 1 means merge is active, 0 merge is disabled, should be used only with animation mode 2, will merge all models + skeleton + animation in chosen file to 1 model/Scene 
CRC_CORRECTION = 0                  #Attemet fo fix CRC for modified files

#Mesh related options
SKIP_MESH = 0                       #Use file mesh.txt to skip mesh loading: 1 - skip by mesh name, 2- skip by start with string , 3 skip by end with string
MESH_OPTIMIZE = 0                   #optimze mesh, remove duplicte vertcies etc..
S_NORMALS = 0                       #Smooth Normals
DEBUG_NORMALS = 0                   #Visuelize Normals as colors
LOAD_POINT_CLOUD = 0                #Load point cloud (Only vertcies) of mesh

#Specific Game related Options
DISABLE_ALPHA = True                #Disable Alpha channel when loading Textures
GAME_TAG_GRANNY = True              #Enable AutoTexturing for GR2 files eith Embedded Textures
GAME_TAG_ESO = 0                    #Needs to be set to 1 in order to get correct UV for ESO Static meshes
GAME_TAG_BG3 = 0                    #Enable AutoTexturing for BG3
GAME_TAG_SH5 = 0                    #Enable AutoTexturing for SH5

#Transform Options - Wanted Orination
TRANSFORM_FILE = True
RightVector =   [1,0,0]
UpVector =      [0,1,0]
BackVector =    [0,0,1]
UnitsPerMeter_new = 1.0

#Glabal system Variable
SysEnvironment  = 8 * struct.calcsize("P")

BG3_PATH = 'D:/SteamLibrary/steamapps/common/Baldurs Gate 3/Data/'


def registerNoesisTypes():
    handle = noesis.register("GR2 Reader", ".gr2;.fgx;.model;.skeleton")
    noesis.setHandlerTypeCheck(handle, noepyCheckType)
    noesis.setHandlerLoadModel(handle, noepyLoadModel)
      
    handle = noesis.register("Bauldrs Gate 3 Archive", ".pak")
    noesis.setHandlerExtractArc(handle, extractPAK)
    #opens debug consle
    noesis.logPopup()
    return 1


def noepyCheckType(data):
    '''Verify that the format is supported by this plugin. Default yes'''
    #I preform my check while reading mesh data
    return 1
    


#===========================================================
#Transform Coordinate System Functions
#===========================================================

def CalculateDeterminant (Linear3x3):
      
      Matrix = []
      for i in range(3):
        for j in range(3):
            Matrix.append(Linear3x3[i][j])
            
      det = ((Matrix[4] * Matrix[8] - Matrix[5] * Matrix[7]) * Matrix[0]) - ((Matrix[3] * Matrix[8] - Matrix[6] * Matrix[5]) * Matrix[1]) + ((Matrix[3] * Matrix[7] - Matrix[6] * Matrix[4]) * Matrix[2])
      
      return det


def TranformVertcies (Linear3x3,Positions,Normals,Tangent,Binormal,Affine3,UnitsPerMeter_old,UnitsPerMeter_new):
    NewPositions = []
    Matrix = []
    
    for i in range(3):
      for j in range(3):
        Matrix.append(Linear3x3[i][j])
        
    VertCount = len(Positions)
    
    for i in range(VertCount):
        px = Matrix[1] * Positions[i][1] + Matrix[0] * Positions[i][0] + Matrix[2] * Positions[i][2] + Affine3[0]
        py = Matrix[4] * Positions[i][1] + Matrix[3] * Positions[i][0] + Matrix[5] * Positions[i][2] + Affine3[1]
        pz = Matrix[7] * Positions[i][1] + Matrix[6] * Positions[i][0] + Matrix[8] * Positions[i][2] + Affine3[2]
        Positions[i][0] = px
        Positions[i][1] = py
        Positions[i][2] = pz

        if Normals:
            #multiply by UnitsPerMeter_old inorder to renormlize normals to -1 - 1, instead of normlizing by the book
            nx = Matrix[1] * Normals[i][1] + Matrix[0] * Normals[i][0] + Matrix[2] * Normals[i][2]# + Affine3[0]
            ny = Matrix[4] * Normals[i][1] + Matrix[3] * Normals[i][0] + Matrix[5] * Normals[i][2]# + Affine3[1]
            nz = Matrix[7] * Normals[i][1] + Matrix[6] * Normals[i][0] + Matrix[8] * Normals[i][2]# + Affine3[2]
            Normals[i][0] = nx * UnitsPerMeter_old/UnitsPerMeter_new
            Normals[i][1] = ny * UnitsPerMeter_old/UnitsPerMeter_new
            Normals[i][2] = nz * UnitsPerMeter_old/UnitsPerMeter_new

        if Tangent:
            Tanx = Matrix[1] * Tangent[i][1] + Matrix[0] * Tangent[i][0] + Matrix[2] * Tangent[i][2]# + Affine3[0]
            Tany = Matrix[4] * Tangent[i][1] + Matrix[3] * Tangent[i][0] + Matrix[5] * Tangent[i][2]# + Affine3[1]
            Tanz = Matrix[7] * Tangent[i][1] + Matrix[6] * Tangent[i][0] + Matrix[8] * Tangent[i][2]# + Affine3[2]
            Tangent[i][0] = Tanx #* UnitsPerMeter_old/UnitsPerMeter_new
            Tangent[i][1] = Tany #* UnitsPerMeter_old/UnitsPerMeter_new
            Tangent[i][2] = Tanz #* UnitsPerMeter_old/UnitsPerMeter_new
               
        if Binormal:
            binx = Matrix[1] * Binormal[i][1] + Matrix[0] * Binormal[i][0] + Matrix[2] * Binormal[i][2]# + Affine3[0]
            biny = Matrix[4] * Binormal[i][1] + Matrix[3] * Binormal[i][0] + Matrix[5] * Binormal[i][2]# + Affine3[1]
            binz = Matrix[7] * Binormal[i][1] + Matrix[6] * Binormal[i][0] + Matrix[8] * Binormal[i][2]# + Affine3[2]
            Binormal[i][0] = binx
            Binormal[i][1] = biny
            Binormal[i][2] = binz

    return


def RewindIndcies (Indices):
    IndicesCount = len(Indices)
    for i in range(IndicesCount//3):
        Indices[i * 3 + 2],Indices[i * 3] = Indices[i * 3],Indices[i * 3 + 2]


def TransformPositions (Positions,Linear3x3,Affine3):

    px = Linear3x3[1] * Positions[1] + Linear3x3[0] * Positions[0] + Linear3x3[2] * Positions[2]# + Affine3[0]
    py = Linear3x3[4] * Positions[1] + Linear3x3[3] * Positions[0] + Linear3x3[5] * Positions[2]# + Affine3[1]
    pz = Linear3x3[7] * Positions[1] + Linear3x3[6] * Positions[0] + Linear3x3[8] * Positions[2]# + Affine3[2]
    Positions[0] = px
    Positions[1] = py
    Positions[2] = pz
    
    return Positions


def TransformScaleShear(ScaleShear,Linear3x3,InverseLinear3x3,Affine3):

    A =  NoeVec3((ScaleShear[0],ScaleShear[1],ScaleShear[2]))
    B =  NoeVec3((ScaleShear[3],ScaleShear[4],ScaleShear[5]))
    C =  NoeVec3((ScaleShear[6],ScaleShear[7],ScaleShear[8]))
    D =  NoeVec3((0,0,0))

    NoaScaleShear = NoeMat43([A,B,C,D])
    
    E =  NoeVec3((Linear3x3[0],Linear3x3[1],Linear3x3[2]))
    F =  NoeVec3((Linear3x3[3],Linear3x3[4],Linear3x3[5]))
    G =  NoeVec3((Linear3x3[6],Linear3x3[7],Linear3x3[8]))
    H =  NoeVec3((0,0,0))
    
    NoaLinear3x3 = NoeMat43([E,F,G,H])
    
    NoaInverseLinear3x3 = NoaLinear3x3.inverse()
    composite = NoaScaleShear * NoaLinear3x3
    composite = NoaInverseLinear3x3 * composite
    
    scale = []
    
    for i in range(3):
        for j in range(3):
            scale.append(composite[i][j])
            
    return scale


def TransformOrientation(Orientation,Linear3x3,InverseLinear3x3):

            Matrix = NoeQuat((Orientation[0],Orientation[1],Orientation[2],Orientation[3])).normalize().toMat43()
            
            E =  NoeVec3((Linear3x3[0],Linear3x3[1],Linear3x3[2]))
            F =  NoeVec3((Linear3x3[3],Linear3x3[4],Linear3x3[5]))
            G =  NoeVec3((Linear3x3[6],Linear3x3[7],Linear3x3[8]))
            H =  NoeVec3((0,0,0))
    
            NoaLinear3x3 = NoeMat43([E,F,G,H])
            NoaInverseLinear3x3 = NoaLinear3x3.inverse()
            composite = Matrix * NoaLinear3x3
            composite = NoaInverseLinear3x3 * composite
            NoaOrientation= composite.toQuat()
            
            return list(NoaOrientation)


def TransformSkeleton(Linear3x3,InverseLinear3x3, Affine3,InverseWorldTransform,UnitsPerMeter_old,UnitsPerMeter_new):
    
    Linear4x4 = NoeMat44( [(Linear3x3[0][0],Linear3x3[0][1],Linear3x3[0][2],0.0),
                           (Linear3x3[1][0],Linear3x3[1][1],Linear3x3[1][2],0.0),
                           (Linear3x3[2][0],Linear3x3[2][1],Linear3x3[2][2],0.0),
                           (0.0,0.0,0.0,1.0)])
                             
    InverseLinear4x4 = NoeMat44( [(InverseLinear3x3[0][0],InverseLinear3x3[0][1],InverseLinear3x3[0][2],0.0),
                                  (InverseLinear3x3[1][0],InverseLinear3x3[1][1],InverseLinear3x3[1][2],0.0),
                                  (InverseLinear3x3[2][0],InverseLinear3x3[2][1],InverseLinear3x3[2][2],0.0),
                                  (0.0,0.0,0.0,1.0)])

    BoneCount = len(InverseWorldTransform)
    
    #Transform InverseWorldTransform to new coordinate system, scale and origin
    for i in range(BoneCount):
        InverseWorldTransform[i] = NoeMat44( [(InverseWorldTransform[i][0],InverseWorldTransform[i][1],InverseWorldTransform[i][2],InverseWorldTransform[i][3]),
                          (InverseWorldTransform[i][4],InverseWorldTransform[i][5],InverseWorldTransform[i][6],InverseWorldTransform[i][7]), 
                          (InverseWorldTransform[i][8],InverseWorldTransform[i][9],InverseWorldTransform[i][10],InverseWorldTransform[i][11]), 
                          (InverseWorldTransform[i][12],InverseWorldTransform[i][13],InverseWorldTransform[i][14],InverseWorldTransform[i][15])] )
                     
        Matrix = Linear4x4 * InverseWorldTransform[i] * InverseLinear4x4
        Temp = []
        for j in range(4):
            for k in range(4):
                Temp.append(Matrix[j][k])
        
        #fix the translation scale and origin, sclae needs fixing due to InverseLiner3x3x
        SizeFix = (UnitsPerMeter_new * UnitsPerMeter_new)/(UnitsPerMeter_old * UnitsPerMeter_old)
        
        Temp[12] = (Temp[12] * SizeFix) + Affine3[0]
        Temp[13] = (Temp[13] * SizeFix) + Affine3[1]
        Temp[14] = (Temp[14] * SizeFix) + Affine3[2]
        InverseWorldTransform[i] = Temp

        
    return
                        
                        
def TransformSkeletonLocal(MatrixA,InverseMatrixA,Loacl_Transform,Affine3):
                    
        Linear3x3 = []
        InverseLinear3x3 = []
        
        for i in range(3):
            for j in range(3):
                Linear3x3.append(MatrixA[i][j])
                InverseLinear3x3.append(InverseMatrixA[i][j])
                
        Count = len(Loacl_Transform)
        
        for i in range(Count):
            Loacl_Transform[i].Translation = TransformPositions(Loacl_Transform[i].Translation,Linear3x3,Affine3)
            Loacl_Transform[i].ScaleShear = TransformScaleShear(Loacl_Transform[i].ScaleShear,Linear3x3,InverseLinear3x3,Affine3)
            Loacl_Transform[i].Quaterion = TransformOrientation(Loacl_Transform[i].Quaterion,Linear3x3,InverseLinear3x3)
            
        return
                            
                            
def TransformTrackGroupSystem(Loacl_Transform,Type,Linear3x3,InverseLinear3x3,Affine3):

    if Type == 'Position':
        Transformed_Positions = []
        
        for Positions in Loacl_Transform:
            Transformed_Positions.append(TransformPositions (Positions,Linear3x3,Affine3))
            
        return Transformed_Positions

    if Type == 'ScaleShear':
        Transformed_ScaleShear = []
        
        for ScaleShear in Loacl_Transform:
            Transformed_ScaleShear.append(TransformScaleShear(ScaleShear,Linear3x3,InverseLinear3x3,Affine3))
            
        return Transformed_ScaleShear

    if Type == 'Rotation':
        Transformed_Orientation = []
        
        for Orientation in Loacl_Transform:
            Transformed_Orientation.append(TransformOrientation(Orientation,Linear3x3,InverseLinear3x3))
            
        return Transformed_Orientation
            
            
    if Type == 'InitialPlacement':
        Transformed_InitialPlacement = []

        for i in range(3):
            for j in range(3):
                Transformed_InitialPlacement.append(Loacl_Transform[i][j])
                
        Transformed_InitialPlacement= TransformScaleShear(Transformed_InitialPlacement,Linear3x3,InverseLinear3x3,Affine3)
        Transformed_Positions = TransformPositions (Loacl_Transform[3],Linear3x3,Affine3)
        A = NoeVec3([Transformed_InitialPlacement[0],Transformed_InitialPlacement[1],Transformed_InitialPlacement[2]])
        B = NoeVec3([Transformed_InitialPlacement[3],Transformed_InitialPlacement[4],Transformed_InitialPlacement[5]])
        C = NoeVec3([Transformed_InitialPlacement[6],Transformed_InitialPlacement[7],Transformed_InitialPlacement[8]])
        D = NoeVec3([Transformed_Positions[0],Transformed_Positions[1],Transformed_Positions[2]])
        Transformed_InitialPlacement = NoeMat43([A,B,C,D])

        return Transformed_InitialPlacement


#===========================================================
#Extract Silent Hunter 5 Texture Functions
#===========================================================

class SH_Material:
    def __init__(self):
        self.Name = ''
        self.Maps = []


class SH_Maps:
    def __init__(self):
        self.Usage = ''
        self.Name = ''
        self.Textue_Name = ''
        self.Width = ''
        self.Height = ''
        self.Encoding = ''
        self.SubFormat = ''
        self.BytesPerPixel = ''
        self.Shift = ''
        self.Bits = ''
        self.TextureType = ''
        self.MIPLevels = ''


def ProcessMapsSH(ma,new_Mat):
    new_Map = SH_Maps()
    new_Mat.Maps.append(new_Map)
    new_Map.Usage = ma.Usage
    new_Map.Name = ma.Map.Name
    
    if ma.Map.Texture:
        new_Map.Textue_Name = ma.Map.Texture.FromFileName
        new_Map.Width = ma.Map.Texture.Width
        new_Map.Height = ma.Map.Texture.Height
        new_Map.Encoding = ma.Map.Texture.Encoding
        new_Map.SubFormat = ma.Map.Texture.SubFormat
        new_Map.TextureType = ma.Map.Texture.TextureType

        if ma.Map.Texture.Layout:
            new_Map.BytesPerPixel = ma.Map.Texture.Layout.BytesPerPixel
            new_Map.Shift = ma.Map.Texture.Layout.ShiftForComponent
            new_Map.Bits = ma.Map.Texture.Layout.BitsForComponent
            
        if ma.Map.Texture.Images and ma.Map.Texture.Images.MIPLevels:
            new_Map.MIPLevels = ma.Map.Texture.Images.MIPLevels

    if ma.Map.Maps:
        get_SH_maps(ma.Map.Maps,new_Mat)


def get_SH_maps(Maps,new_Mat):
    if type(Maps) == list:
        for ma in Maps:
            ProcessMapsSH(ma,new_Mat)
    else:
        ProcessMapsSH(Maps,new_Mat)


class granny_pixel_layout(ctypes.Structure):
    _fields_ = [
        ('BytesPerPixel', ctypes.c_int32),
        ('ShiftForComponent', (ctypes.c_int32 * 4)),
        ('BitsForComponent', (ctypes.c_int32 * 4))
    ]

def BinkDecompress(Texture,Source):

    Encoding = Texture.Encoding
    FromFileName = Texture.Textue_Name
    Height = Texture.Height
    ShiftForComponent = Texture.Shift
    SubFormat = Texture.SubFormat
    TextureType = Texture.TextureType
    Width = Texture.Width
    BytesPerPixel = Texture.BytesPerPixel
    BitsForComponent = Texture.Bits
    DestStride = Width*BytesPerPixel
    SourceSize = len(Source)
    
    if SysEnvironment == 32:
        lib = ctypes.WinDLL("granny2.dll")
        GetRawImageSize = lib['_GrannyGetRawImageSize@16']
        BinkDecompressTexture = lib['_GrannyBinkDecompressTexture@32']
    else:
        lib = ctypes.WinDLL("granny2_x64.dll")
        GetRawImageSize = lib['GrannyGetRawImageSize']
        BinkDecompressTexture = lib['GrannyBinkDecompressTexture']
        
    Layout = granny_pixel_layout()
    GetRawImageSize.argtypes = (ctypes.POINTER(granny_pixel_layout),ctypes.c_int32, ctypes.c_int32, ctypes.c_int32)
    GetRawImageSize.restype = ctypes.c_int32
    value = GetRawImageSize(ctypes.byref(Layout),Width * BytesPerPixel ,Width,Height)

    Dest = bytes(value)
    DestLayout = granny_pixel_layout()
    Source = bytes(Source)
    DestLayout.BytesPerPixel = BytesPerPixel
    DestLayout.ShiftForComponent[0] = ShiftForComponent[0]
    DestLayout.ShiftForComponent[1] = ShiftForComponent[1]
    DestLayout.ShiftForComponent[2] = ShiftForComponent[2]
    DestLayout.ShiftForComponent[3] = ShiftForComponent[3]
    DestLayout.BitsForComponent[0] = BitsForComponent[0]
    DestLayout.BitsForComponent[1] = BitsForComponent[1]
    DestLayout.BitsForComponent[2] = BitsForComponent[2]
    DestLayout.BitsForComponent[3] = BitsForComponent[3]
    BinkDecompressTexture.argtypes =(ctypes.c_int32 , ctypes.c_int32, ctypes.c_uint32 , ctypes.c_int32, ctypes.c_void_p, ctypes.POINTER(granny_pixel_layout), ctypes.c_int32 ,ctypes.c_void_p)
    BinkDecompressTexture.restype = ctypes.c_void_p
    BinkDecompressTexture(Width, Height, SubFormat, SourceSize, Source, DestLayout, DestStride, Dest)
    
    return Dest
                    

def GetGrannyTextures(Textures, Mesh_Name, MatCount,texListCurrent):
        
        texList = []
        material = []

        for index,tex_map in enumerate(Textures):

            if (not tex_map.Textue_Name and not tex_map.Encoding) or (not tex_map.MIPLevels):
                continue

            flag = 0
            name = tex_map.Textue_Name
            Width = tex_map.Width
            Height = tex_map.Height
            BytesPerPixel = tex_map.BytesPerPixel
            if type(tex_map.MIPLevels) == list:
                imgData = []
                for i in range(len(tex_map.MIPLevels)):
                    if hasattr(tex_map.MIPLevels[i],"PixelBytes"):
                        imgData += tex_map.MIPLevels[i].PixelBytes
                    elif hasattr(tex_map.MIPLevels[i],"Pixels"):
                        imgData += tex_map.MIPLevels[i].Pixels
                data = bytearray(imgData)

            else:   
                if hasattr(tex_map.MIPLevels, "PixelBytes"):
                    data = bytearray(tex_map.MIPLevels.PixelBytes)
                if hasattr(tex_map.MIPLevels, "Pixels"):
                    data = bytearray(tex_map.MIPLevels.Pixels)

            #Check if we already loaded this texture, if yes just set name to material
            if texListCurrent:
                 for Tc in texListCurrent:
                    if name == Tc.name:
                        flag = 1
                        break
                        
            if texList:
                 for Tc in texList:
                    if name == Tc.name:
                        flag = 1
                        break

            if tex_map.Encoding == 1: #RGBA32
                texFmt = noesis.NOESISTEX_RGBA32
                
            if tex_map.Encoding == 2: #DXT
                if tex_map.SubFormat == 0:
                    texFmt = noesis.FOURCC_DXT1
                if tex_map.SubFormat == 1:
                    texFmt = noesis.FOURCC_DXT1NORMAL
                if tex_map.SubFormat == 2:
                    texFmt = noesis.FOURCC_DXT3
                if tex_map.SubFormat == 3:
                    texFmt = noesis.FOURCC_DXT5

                if flag != 1:
                    data = rapi.imageDecodeDXT(data, Width, Height, texFmt)
                    texFmt = noesis.NOESISTEX_RGBA32
                    
            if tex_map.Encoding == 3: #BINK
                data = BinkDecompress(tex_map,data)
                if BytesPerPixel == 4:
                    texFmt = noesis.NOESISTEX_RGBA32
                elif BytesPerPixel == 3:
                    texFmt = noesis.NOESISTEX_RGB24
                
            if flag != 1:
                tex = NoeTexture(name,Width, Height, data, texFmt)
                texList.append(tex)
            
            if not material:
                material = NoeMaterial("Material_" + Mesh_Name +str(MatCount),name)
                if DISABLE_ALPHA:
                    material.setDefaultBlend(0)

            if tex_map.Usage == 'Diffuse Color':
                material.setTexture(name)
                
            if tex_map.Usage == 'Self-Illumination':
                material.setOcclTexture(name)
                
            if tex_map.Usage == 'Bump' or tex_map.Usage == 'Additional Bumpmap':
                material.setNormalTexture(name)
                    
            if tex_map.Usage == 'Opacity':
                material.setOpacityTexture(name)

        return texList ,material
        
def GetTexturesSH5(textureNames, Mesh_Name, MatCount,texListCurrent):

        texList = []
        material = []

        PATH = rapi.getDirForFilePath(rapi.getInputName())

        for index,tex_map in enumerate(textureNames):
            flag = 0
            baseName = rapi.getLocalFileName(tex_map.Textue_Name).lower()
            if not tex_map.Textue_Name:
                continue
            
            #Check if we already loaded this texture, if yes just set name to material
            if texListCurrent:
                 for Tc in texListCurrent:
                    if baseName[:-4] == Tc.name:
                        flag = 1
                        name = baseName[:-4]
                        break

            if rapi.checkFileExists(PATH + baseName):
                fs = open(PATH + baseName, 'rb')
                data = fs.read()
                Extension = baseName[-4:]
                
                #try:
                if flag != 1:
                    tex = rapi.loadTexByHandler(data, Extension)
                    tex.name = baseName[:-4]
                    texList.append(tex)
                    name = tex.name

                if not material:
                    material = NoeMaterial("Material_" + Mesh_Name +str(MatCount),name)
                    #Disable alpha - becuae AO map is all black and turning ship trsnsperent 
                    if DISABLE_ALPHA:
                        material.setDefaultBlend(0)

                if tex_map.Usage == 'Diffuse Color':
                    material.setTexture(name)
                    
                if tex_map.Usage == 'Self-Illumination':
                    material.setOcclTexture(name)
                    
                if tex_map.Usage == 'Bump' or tex_map.Usage == 'Additional Bumpmap':
                    material.setNormalTexture(name)
 
        return texList ,material
                
                
#===========================================================
#Extract Diablo II: Resurrected Texture Functions
#===========================================================

def GetTexturesDS2(textureNames, Mesh_Name, MatCount,texListCurrent):

    texList = []
    material = []
            
    class MipInfo :

      def __init__(self):
            self.start = 0
            self.size = 0

    PATH = rapi.getDirForFilePath(rapi.getInputName()) + "textures\\" 

    for tex in textureNames:
        
        MipData = []
        imgData = []
        TextureName = []
        flag = 0
        
        baseNameExt = rapi.getLocalFileName(tex).lower() #with extension
        baseName = rapi.getExtensionlessName(rapi.getLocalFileName(tex)).lower() #no extension
        
         
        #check if we already have the texture loaded, if yes just set it to material
        if texListCurrent:            
             for Tc in texListCurrent:
                if baseName == Tc.name:
                    flag = 1
                    TextureName = baseName
                    break
        
        
        if flag != 1:

            if rapi.checkFileExists(PATH + baseNameExt):
                                  
                g = open(PATH + baseNameExt, 'rb')
                data = g.read()
                f = NoeBitStream(data, NOE_LITTLEENDIAN)
    
                MagicValue = f.readUInt()
                Type = f.readUShort()
                unk = f.readUShort()
                imgWidth = f.readUInt()
                imgHeight = f.readUInt()
                depth = f.readUInt()        
                f.seek(8, 1)
                MipLevels = f.readUInt()
                Channel_Count = f.readUInt()
               
                for lvl in range(MipLevels):
                    Temp = MipInfo ()
                    Temp.size = f.readUInt()
                    Current = f.tell() 
                    Temp.start = f.readUInt()
                    Temp.start += Current
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
                    data = rapi.imageDecodeDXT(data, imgWidth, imgHeight, noesis.FOURCC_ATI1)
                    texFmt = noesis.NOESISTEX_RGBA32


                tex = NoeTexture(baseName, imgWidth, imgHeight, data, texFmt)                                     
                TextureName =  tex.name      
        
        #When trying to open file script didnt find it, and it dosent exist already in texList
        if not TextureName:
            continue
        
        if not material:
            material = NoeMaterial("Material_" + Mesh_Name +str(MatCount),TextureName)


                         
        if baseName[-3:] == 'alb':
            if flag != 1: 
                texList.append(tex)
            material.setTexture(TextureName)
            
        if baseName[-3:] == 'nrm':
            if flag != 1:  
                rgba = rapi.imageGetTexRGBA(tex)
                #swap red and alpha channels
                rgba = rapi.imageNormalSwizzle(rgba, tex.width, tex.height, 1, 1, 0)
                tex.pixelData = rgba
                tex.pixelType = noesis.NOESISTEX_RGBA32
                tex.mipCount = 0                        
                texList.append(tex)
            material.setNormalTexture(TextureName)
            
    return texList ,material


#===========================================================
#Extract BG3 Texture Functions
#===========================================================


class MeshMaterial:
    def __init__(self):
        self.MatrialID = ''
        self.MeshName = []
        self.TextureID = []
        self.TexturePath = []

class TextureID:

    def __init__(self):
        self.MatrialID = ''
        self.MeshName = ''
        self.TextureID = ''
        self.StartIndex = ''
        self.EndIndex = ''
        self.TexturePath = ''

    def __eq__(self, other) :
            if self.MatrialID == other.MatrialID: 
                return True
            else:
                return False

    def __lt__(self, other):
         return self.MatrialID < other.MatrialID


#return false in contains any texture id and True if only ''
def SearchTexID(TexID):
    for tx in TexID:
        if tx != '':
            return False

    return True


def FindSubStringInList(sub,list):
    for s in list:
        if sub in s:
            return True

    return False



def getTexturesID(data):

    MeshDataID = []
    TextureMapID = []

    MeshIndex = [m.start() for m in re.finditer(r'_Mesh', data)]

    for j in range(len(MeshIndex)):

        NewID = TextureID()

        End_index = MeshIndex[j] + 5

        i = 0
        while data[End_index] != '.':
            End_index += 1

        for i in range(len(data)):
            if data[MeshIndex[j]-i] == '\x00': 
                Start_index = MeshIndex[j] - i + 1
                break

        NewID.MeshName = data[Start_index :End_index]

        Start_index -= 1
        NewID.MatrialID = data[Start_index-36: Start_index]

        MeshDataID.append(NewID)

    IndexList1 = [m.start() for m in re.finditer(r'Texture Map', data)]
    
    for mesh in MeshDataID:

        MaterialID_Index = [m.start() for m in re.finditer(mesh.MatrialID, data)]
        
        #find first 'texture map' after MatrialID index, i assume the first location of MatrialID is for texture map
        idx = bisect.bisect(IndexList1, MaterialID_Index[0])
        
        IndexRange = len(IndexList1) - (idx)

        if IndexRange == 0:
            NewID = TextureID()
            NewID.MatrialID = mesh.MatrialID
            NewID.MeshName = mesh.MeshName
            TextureMapID.append(NewID)  


        for id in range(IndexRange):
                                              
            start = IndexList1[idx + id]
           
            i = 0
            while data[start + i] != '\x00':  
                i += 1
            start = start + i + 1

            i = 0
            while data[start + i] != '\x00':  
                i += 1   
            
            end = start +  i

            NewID = TextureID()
            NewID.StartIndex = start +12
            NewID.EndIndex = end
            NewID.TextureID = data[start:end]
            NewID.MatrialID = mesh.MatrialID
            NewID.MeshName = mesh.MeshName
            TextureMapID.append(NewID)                                   


            if (len(IndexList1) - 1) >= (idx + id + 1) and 'Vector'  in data[IndexList1[idx + id] : IndexList1[idx + id + 1]]:
                break
                                
    for tx in TextureMapID:
        if tx.TextureID:
            ddsID = [m.start() for m in re.finditer(tx.TextureID, data)]
            if len(ddsID) == 1:
                continue

            for ln in ddsID:
                if ln == tx.StartIndex:
                    continue
                else:
                    stratIndex = ln +len(tx.TextureID) + 2
                    #the +2 in PathStart is because for files with .dds they start with \x00\00
                    PathStart = data[stratIndex + data[stratIndex:].find('\x00') + 2:]                                                        
                    tx.TexturePath = BG3_PATH + 'Textures_files/' + PathStart[:PathStart.find('\x00')]

                    if tx.TexturePath[-3:] != 'DDS':
                        tx.TexturePath = ''


    return TextureMapID



def getLSFfile(meshName, LSF_Files_Array):

    for root, dirs, files in os.walk(BG3_PATH):
        for fileName in files:
            if fileName == '_merged.lsf':
                lsffile = os.path.join(root, fileName)        

                if lsffile in LSF_Files_Array:
                    continue

                with io.open(lsffile, encoding='utf-8', errors='ignore') as f:
                    data = f.read()
                 
                    if meshName in data:
                        LSF_Files_Array.append(lsffile)
                        f.seek(0,0)

                        return LSF_Files_Array, data

    return LSF_Files_Array, None



def ConstructMaterials(data, Temp):
    if not data:
        return None

    TextureMapID = getTexturesID(data)
       
    i = 0    
    if TextureMapID:
        TextureMapID.sort()
        MatID = TextureMapID[i].MatrialID
        Data = MeshMaterial()
        Data.MatrialID = MatID
        while i < len(TextureMapID):

            if TextureMapID[i].MatrialID !=  MatID:
                Temp.append(Data)
                Data = MeshMaterial()
                MatID = TextureMapID[i].MatrialID
                Data.MatrialID = TextureMapID[i].MatrialID

            if TextureMapID[i].TextureID not in Data.TextureID:
                Data.TextureID.append(TextureMapID[i].TextureID)
            if TextureMapID[i].TexturePath not in Data.TexturePath:
                Data.TexturePath.append(TextureMapID[i].TexturePath)
            if TextureMapID[i].MeshName not in Data.MeshName:
                Data.MeshName.append(TextureMapID[i].MeshName)
            i += 1     

        Temp.append(Data)

        return Temp




def SeperateChannels(TextData):

    #seperate all channels
    red = bytearray()
    green = bytearray()
    blue = bytearray()
    alpha = bytearray()
    for i in range(len(TextData)//4):
            red.append(TextData[i*4])
            green.append(TextData[i*4 + 1])
            blue.append(TextData[i*4 + 2])
            alpha.append(TextData[i*4 + 3])
    
    return red, green, blue,alpha


    
def InvertGreenChannel(TextData):
   
    for i in range(len(TextData)//4):         
        TextData[i*4 + 1] = 255 - TextData[i*4 + 1]

    return TextData
   

   
def GetTextures(index,AllMaterials,Mesh_Name, All_texList):
        

        texList = []
        matList = []
        material = []
        FileList = []
        
        for meshMat in AllMaterials:
            if FindSubStringInList(Mesh_Name,meshMat.MeshName):
                FileList = meshMat
                
                LoadedTextures = []
                
                for i in range(len(FileList.TexturePath)): 
                    
                    if FileList.TexturePath[i]:

                        baseName = rapi.getLocalFileName(FileList.TexturePath[i])#.lower()
                        
                        #check if we alraedy have this texture loaded
                        if All_texList:
                            for TexFile in All_texList:
                                if baseName[:-4] == TexFile.name:
                                
                                    LoadedTextures.append(baseName)

                                    if not material:
                                        material = NoeMaterial(Mesh_Name ,TexFile.name)                                  
                                        matList.append(material)
                                        
                                    #BM (Basecolor Map) - Albedo ,Diffuse
                                    if baseName[-6:] == 'BM.DDS' or baseName[-7:] == 'BMA.DDS':
                                        material.setTexture(TexFile.name)
                                  
                                    #NM - normal map with Red channel in Alpha and inverted Green
                                    if baseName[-6:] == 'NM.DDS':
                                        rgba = rapi.imageGetTexRGBA(tex)    
                                        #swap red and alpha channels
                                        rgba = rapi.imageNormalSwizzle(rgba, tex.width, tex.height, 1, 1, 0)
                                        rgba = InvertGreenChannel(rgba) 
                                        tex.pixelData = rgba
                                        tex.pixelType = noesis.NOESISTEX_RGBA32
                                        tex.mipCount = 0
                                        material.setNormalTexture(TexFile.name)

                        #if texture dosent exist in textList                           
                        if rapi.checkFileExists(FileList.TexturePath[i]) and baseName not in LoadedTextures:
                        
                            g = open(FileList.TexturePath[i], 'rb')
                            data = g.read()
                            Extension = baseName[-4:]
                    
                            try:
                                tex = rapi.loadTexByHandler(data, Extension)
                                tex.name = baseName[:-4]
                                texList.append(tex)
                                if not material:
                                    material = NoeMaterial(Mesh_Name ,tex.name)                                  
                                    matList.append(material)
                                    
                                #BM (Basecolor Map) - Albedo ,Diffuse
                                if baseName[-6:] == 'BM.DDS' or baseName[-7:] == 'BMA.DDS':
                                    material.setTexture(tex.name)
                              
                                #NM - normal map with Red channel in Alpha and inverted Green
                                if baseName[-6:] == 'NM.DDS':
                                    rgba = rapi.imageGetTexRGBA(tex)    
                                    #swap red and alpha channels
                                    rgba = rapi.imageNormalSwizzle(rgba, tex.width, tex.height, 1, 1, 0)
                                    rgba = InvertGreenChannel(rgba) 
                                    tex.pixelData = rgba
                                    tex.pixelType = noesis.NOESISTEX_RGBA32
                                    tex.mipCount = 0
                                    material.setNormalTexture(tex.name)

                            except:
                                print("failed to load .texture file " + baseName )

        return texList ,matList
        
        
#===========================================================
#Extract BG3 PAK Functions
#===========================================================


#get file and directory names from string 
def getfilename(name):
    array = name.split('/')
    filename = array[len(array)-1]
    num = len(name) - len(filename)
    directoryname = name[:num]

    return filename, directoryname

    
def extractPAK(fileName, fileLen, justChecking):
                   
    with open(fileName, "rb") as f:
        
        TexFile = 0
        
        if justChecking:
            return 1
                
        f.seek(4,0) # magic
        version = struct.unpack("<I",f.read(4))[0] 

        #Get file names and parameters
        table_offset = struct.unpack("<Q",f.read(8))[0]
        f.seek(table_offset,0)
        NumFiles = struct.unpack("<I",f.read(4))[0] 
        CompressedSize = struct.unpack("<I",f.read(4))[0] 

        #Save current location
        TableOffset = f.tell()
        UncompressedSize = NumFiles * 296
        
        #Decompress using lz4.block format
        DecompressedData = rapi.decompLZ4(f.read(CompressedSize),UncompressedSize)

        #Convert data to IO format       
        DecompressStream = NoeBitStream(bytes(DecompressedData), NOE_LITTLEENDIAN)
        
        for i in range(NumFiles): 
            string = noeStrFromBytes(DecompressStream.readBytes(256), "UTF8")
            offset = DecompressStream.readUInt64()
            CompressedSize = DecompressStream.readUInt64()
            UncompressedSize = DecompressStream.readUInt64()
            dummy = DecompressStream.readUInt64()
            crc = DecompressStream.readUInt()
            dummy = DecompressStream.readUInt()
            
            if i > 0 and offset == 0:
                TexFile += 1
                Num = str(TexFile)
                f.close()
                f = open("D:\\SteamLibrary\\steamapps\\common\\Baldurs Gate 3\\Data\\Textures" + "_" + Num + ".pak", 'rb')
        
            f.seek(offset,0)
            #if file is compressed

            if UncompressedSize != 0 and fileName[-10:] != 'LowTex.pak':       
                FileDecompressedData = rapi.decompLZ4(f.read(CompressedSize),UncompressedSize)

            else:
                #data is not compressed
                FileDecompressedData = f.read(CompressedSize)

            filename, directoryname = getfilename(string)
            print("{} {}".format(i,directoryname + filename))
            rapi.exportArchiveFile(directoryname + filename , FileDecompressedData)

    return 1 
    
    

#===========================================================
#Supporting Animation Classes
#===========================================================

class dakeyframes32f: # type = 0

    def __init__(self):

         self.Format = 0
         self.Degree = 0
         self.Dimension = 0
         self.Controls = []

    def NumKnots(self):
         return len(self.Controls)//self.Dimension


    def Knots(self):
        knots = []
        for i in range(self.NumKnots()):
            knots.append(float(i))
        return knots

    def GetTranslation(self):
        NumKnots = self.NumKnots()
        Translation = []
        for i in range(NumKnots):
            vector = [None]*3
            vector[0] = self.Controls[i * 3 + 0]
            vector[1] = self.Controls[i * 3 + 1]
            vector[2] = self.Controls[i * 3 + 2]
            Translation.append(vector)

        return Translation, self.Knots()

    def GetMartix(self):
        NumKnots = self.NumKnots()
        matrix = []
        for i in range(NumKnots):
            mat = [None]*9
            mat[0] = self.Controls[i * 9 + 0]
            mat[1] = self.Controls[i * 9 + 1]
            mat[2] = self.Controls[i * 9 + 2]
            mat[3] = self.Controls[i * 9 + 3]
            mat[4] = self.Controls[i * 9 + 4]
            mat[5] = self.Controls[i * 9 + 5]
            mat[6] = self.Controls[i * 9 + 6]
            mat[7] = self.Controls[i * 9 + 7]
            mat[8] = self.Controls[i * 9 + 8]
            matrix.append(mat)

        return matrix, self.Knots()

    def GetQuterions(self):
        NumKnots = self.NumKnots()
        quats = []
        for i in range(NumKnots):
            quat = [None]*4
            quat[0] = self.Controls[i * 4 + 0]
            quat[1] = self.Controls[i * 4 + 1]
            quat[2] = self.Controls[i * 4 + 2]
            quat[3] = self.Controls[i * 4 + 3]
            quats.append(quat)

        return quats, self.Knots()


class dak32fc32f: #type = 1

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Padding = 0
         self.Knots = []
         self.Controls = []


    def NumKnots(self):
         return len(self.Knots)


    def GetTranslation(self): 
        NumKnots = self.NumKnots()
        Translation = []
        for i in range(NumKnots):
            vector = [None]*3
            vector[0] = self.Controls[i * 3 + 0]
            vector[1] = self.Controls[i * 3 + 1]
            vector[2] = self.Controls[i * 3 + 2]
            Translation.append(vector)

        return Translation, self.Knots

    def GetMartix(self):
        NumKnots = self.NumKnots()
        matrix = []
        for i in range(NumKnots):
            mat = [None]*9
            mat[0] = self.Controls[i * 9 + 0]
            mat[1] = self.Controls[i * 9 + 1]
            mat[2] = self.Controls[i * 9 + 2]
            mat[3] = self.Controls[i * 9 + 3]
            mat[4] = self.Controls[i * 9 + 4]
            mat[5] = self.Controls[i * 9 + 5]
            mat[6] = self.Controls[i * 9 + 6]
            mat[7] = self.Controls[i * 9 + 7]
            mat[8] = self.Controls[i * 9 + 8]
            matrix.append(mat)

        return matrix, self.Knots

    def GetQuterions(self):
        NumKnots = self.NumKnots()
        quats = []
        for i in range(NumKnots):
            quat = [None]*4
            quat[0] = self.Controls[i * 4 + 0]
            quat[1] = self.Controls[i * 4 + 1]
            quat[2] = self.Controls[i * 4 + 2]
            quat[3] = self.Controls[i * 4 + 3]
            quats.append(quat)

        return quats, self.Knots


# it isnt correct to return identity values for animation!! , i fix it before applying flat transform list
class daidentity: #type 2

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Dimension = 0

    def NumKnots(self):
        return 1

    def knots(self):
        a = [0] 
        return a

    def GetTranslation(self):
        IdentityPos = [[0.0,0.0,0.0]]
        return IdentityPos, self.knots()
    
    def GetQuterions(self):
        IdentityQuat = [[0.0,0.0,0.0,1.0]]
        return IdentityQuat ,self.knots()
        
    def GetMartix(self):
        IdentityMatrix = [[1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0]]
        return IdentityMatrix, self.knots()


#holds a single constant value. 
class daconstant32f: # type = 3

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Padding = 0
         self.Controls = []

    def NumKnots(self):
        return 1

    def knots(self):
        return [0]

    def GetQuterions(self):
        q = [None]*4
        q[0] = self.Controls[0]
        q[1] = self.Controls[1]
        q[2] = self.Controls[2]
        q[3] = self.Controls[3]

        return [q], self.knots()


    def GetTranslation(self): 
        l = [None]*3
        l[0] = self.Controls[0]
        l[1] = self.Controls[1]
        l[2] = self.Controls[2]

        return [l], self.knots()
        
        
    def GetMartix(self):
        mat = [None]*9
        mat[0] = self.Controls[0]
        mat[1] = self.Controls[1]
        mat[2] = self.Controls[2]
        mat[3] = self.Controls[3]
        mat[4] = self.Controls[4]
        mat[5] = self.Controls[5]
        mat[6] = self.Controls[6]
        mat[7] = self.Controls[7]
        mat[8] = self.Controls[8]

        return [mat], self.knots()


#only stores 3-dimensional data
class d3constant32f: # type = 4

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Padding = 0
         self.Controls = []

    def NumKnots(self):
        return 1

    def knots(self):
        return 0

    def GetTranslation(self):
        v = [None]*3
        v[0] = self.Controls[0]
        v[1] = self.Controls[1]
        v[2] = self.Controls[2]

        Knots = []
        for i in range(self.NumKnots()):
            Knots.append(self.knots())

        return [v], Knots


#only stores 4-dimensional data
class d4constant32f: # type = 5

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Padding = 0
         self.Controls = []

    def NumKnots(self):
        return 1

    def knots(self):
        return [0]

    def GetQuterions(self):
        q = [None]*4
        q[0] = self.Controls[0]
        q[1] = self.Controls[1]
        q[2] = self.Controls[2]
        q[3] = self.Controls[3]
        return [q], self.knots()


class dak16uc16u: # type = 6

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScaleOffsetCount = 0
         self.ControlScaleOffsets = []
         self.KnotsControls = []


    def Components(self):
        return len(self.ControlScaleOffsets)//2

    def NumKnots(self):
        return len(self.KnotsControls)//(self.Components() + 1)

    def Knots(self):
        scale = self.ConvertOneOverKnotScaleTrunc()
        numKnots = self.NumKnots()
        knots = []
        for i in range(numKnots): 
            knots.append(self.KnotsControls[i]/scale)
        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetMartix(self):
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i * 9 + 0] * self.ControlScaleOffsets[0] + self.ControlScaleOffsets[9 + 0]
            mat[1] = self.KnotsControls[numKnots + i * 9 + 1] * self.ControlScaleOffsets[1] + self.ControlScaleOffsets[9 + 1]
            mat[2] = self.KnotsControls[numKnots + i * 9 + 2] * self.ControlScaleOffsets[2] + self.ControlScaleOffsets[9 + 2]
            mat[3] = self.KnotsControls[numKnots + i * 9 + 3] * self.ControlScaleOffsets[3] + self.ControlScaleOffsets[9 + 3]
            mat[4] = self.KnotsControls[numKnots + i * 9 + 4] * self.ControlScaleOffsets[4] + self.ControlScaleOffsets[9 + 4]
            mat[5] = self.KnotsControls[numKnots + i * 9 + 5] * self.ControlScaleOffsets[5] + self.ControlScaleOffsets[9 + 5]
            mat[6] = self.KnotsControls[numKnots + i * 9 + 6] * self.ControlScaleOffsets[6] + self.ControlScaleOffsets[9 + 6]
            mat[7] = self.KnotsControls[numKnots + i * 9 + 7] * self.ControlScaleOffsets[7] + self.ControlScaleOffsets[9 + 7]
            mat[8] = self.KnotsControls[numKnots + i * 9 + 8] * self.ControlScaleOffsets[8] + self.ControlScaleOffsets[9 + 8]
            matrix.append(mat)

        return matrix, self.Knots()

    def GetQuterions(self):
        numKnots = self.NumKnots()
        quats = []
        for i in range(numKnots):
            quat = [None]*4
            quat[0] = self.KnotsControls[numKnots + i * 4 + 0] * self.ControlScaleOffsets[0] + self.ControlScaleOffsets[4 + 0]
            quat[1] = self.KnotsControls[numKnots + i * 4 + 1] * self.ControlScaleOffsets[1] + self.ControlScaleOffsets[4 + 1]
            quat[2] = self.KnotsControls[numKnots + i * 4 + 2] * self.ControlScaleOffsets[2] + self.ControlScaleOffsets[4 + 2]
            quat[3] = self.KnotsControls[numKnots + i * 4 + 3] * self.ControlScaleOffsets[3] + self.ControlScaleOffsets[4 + 3]
            quats.append(quat)

        return quats, self.Knots()
        
        


class dak8uc8u: # type 7

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScaleOffsetCount = 0
         self.ControlScaleOffsets = []
         self.KnotsControls = []

    def Components(self):
        return len(self.ControlScaleOffsets)//2

    def NumKnots(self):
        return len(self.KnotsControls)//(self.Components() + 1)

    def Knots(self):
        scale = self.ConvertOneOverKnotScaleTrunc()
        numKnots = self.NumKnots()
        knots = []
        for i in range(numKnots): 
            knots.append(self.KnotsControls[i]/scale)
        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetMartix(self):
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i * 9 + 0] * self.ControlScaleOffsets[0] + self.ControlScaleOffsets[9 + 0]
            mat[1] = self.KnotsControls[numKnots + i * 9 + 1] * self.ControlScaleOffsets[1] + self.ControlScaleOffsets[9 + 1]
            mat[2] = self.KnotsControls[numKnots + i * 9 + 2] * self.ControlScaleOffsets[2] + self.ControlScaleOffsets[9 + 2]
            mat[3] = self.KnotsControls[numKnots + i * 9 + 3] * self.ControlScaleOffsets[3] + self.ControlScaleOffsets[9 + 3]
            mat[4] = self.KnotsControls[numKnots + i * 9 + 4] * self.ControlScaleOffsets[4] + self.ControlScaleOffsets[9 + 4]
            mat[5] = self.KnotsControls[numKnots + i * 9 + 5] * self.ControlScaleOffsets[5] + self.ControlScaleOffsets[9 + 5]
            mat[6] = self.KnotsControls[numKnots + i * 9 + 6] * self.ControlScaleOffsets[6] + self.ControlScaleOffsets[9 + 6]
            mat[7] = self.KnotsControls[numKnots + i * 9 + 7] * self.ControlScaleOffsets[7] + self.ControlScaleOffsets[9 + 7]
            mat[8] = self.KnotsControls[numKnots + i * 9 + 8] * self.ControlScaleOffsets[8] + self.ControlScaleOffsets[9 + 8]
            matrix.append(mat)

        return matrix, self.Knots()

    def GetQuterions(self):
        numKnots = NumKnots()
        quats = []
        for i in range(numKnots):
            quat = [None]*4
            quat[0] = self.KnotsControls[numKnots + i * 4 + 0] * self.ControlScaleOffsets[0] + self.ControlScaleOffsets[4 + 0]
            quat[1] = self.KnotsControls[numKnots + i * 4 + 1] * self.ControlScaleOffsets[1] + self.ControlScaleOffsets[4 + 1]
            quat[2] = self.KnotsControls[numKnots + i * 4 + 2] * self.ControlScaleOffsets[2] + self.ControlScaleOffsets[4 + 2]
            quat[3] = self.KnotsControls[numKnots + i * 4 + 3] * self.ControlScaleOffsets[3] + self.ControlScaleOffsets[4 + 3]
            quats.append(quat)

        return quats, self.Knots()



class d4nk16uc15u: #type 8

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.ScaleOffsetTableEntries = 0
         self.OneOverKnotScale = 0
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//4

    def Knots(self):
        knots = []
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/self.OneOverKnotScale)   
        return knots


    def CreateQuat(self,a,b,c,scales,offsets):

        swizzle1 = ((b & 0x8000) >> 14) | (c >> 15)
        swizzle2 = (swizzle1 + 1) & 3
        swizzle3 = (swizzle2 + 1) & 3
        swizzle4 = (swizzle3 + 1) & 3

        dataA = (a & 0x7fff) * scales[swizzle2] + offsets[swizzle2]
        dataB = (b & 0x7fff) * scales[swizzle3] + offsets[swizzle3]
        dataC = (c & 0x7fff) * scales[swizzle4] + offsets[swizzle4]

        dataD = math.sqrt((1 - (dataA * dataA + dataB * dataB + dataC * dataC)))
    
        if (a & 0x8000) != 0:
            dataD = -dataD

        quat = [None]*4       
        quat[swizzle2] = dataA
        quat[swizzle3] = dataB
        quat[swizzle4] = dataC
        quat[swizzle1] = dataD

        return quat


    def GetQuterions(self):

        ScaleTable = [
                        1.4142135, 0.70710677, 0.35355338, 0.35355338,
                        0.35355338, 0.17677669, 0.17677669, 0.17677669,
                        -1.4142135, -0.70710677, -0.35355338, -0.35355338,
                        -0.35355338, -0.17677669, -0.17677669, -0.17677669]


        OffsetTable = [
                        -0.70710677, -0.35355338, -0.53033006, -0.17677669,
                        0.17677669, -0.17677669, -0.088388346, 0.0,
                        0.70710677, 0.35355338, 0.53033006, 0.17677669,
                        -0.17677669, 0.17677669, 0.088388346, -0.0]

        knots = self.Knots()

        #now we create quats
        scaleTable = [None]*4
        offsetTable = [None]*4
        selector = self.ScaleOffsetTableEntries

        scaleTable[0] = ScaleTable[(selector >> 0) & 0x0F] * 0.000030518509 
        scaleTable[1] = ScaleTable[(selector >> 4) & 0x0F] * 0.000030518509 
        scaleTable[2] = ScaleTable[(selector >> 8) & 0x0F] * 0.000030518509 
        scaleTable[3] = ScaleTable[(selector >> 12) & 0x0F] * 0.000030518509 

        offsetTable[0] =  OffsetTable[(selector >> 0) & 0x0F]
        offsetTable[1] =  OffsetTable[(selector >> 4) & 0x0F]
        offsetTable[2] =  OffsetTable[(selector >> 8) & 0x0F]
        offsetTable[3] =  OffsetTable[(selector >> 12) & 0x0F]

        numKnots = self.NumKnots()
        quaterions = []
        for i in range(numKnots):          
            quat = self.CreateQuat(self.KnotsControls[numKnots + i * 3 + 0],self.KnotsControls[numKnots + i * 3 + 1],self.KnotsControls[numKnots + i * 3 + 2],scaleTable,offsetTable)
            quaterions.append(quat) #x,y,z,w

        return quaterions,knots 


class d4nk8uc7u: #type 9


    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.ScaleOffsetTableEntries = 0
         self.OneOverKnotScale = 0
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//4

    def Knots(self):
        knots = []
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/self.OneOverKnotScale)

        return knots


    def CreateQuat(self,a,b,c,scales,offsets):

        swizzle1 = ((b & 0x80) >> 6) | ((c & 0x80) >> 7) 
        swizzle2 = (swizzle1 + 1) & 3
        swizzle3 = (swizzle2 + 1) & 3
        swizzle4 = (swizzle3 + 1) & 3

        dataA = (a & 0x7f) * scales[swizzle2] + offsets[swizzle2] 
        dataB = (b & 0x7f) * scales[swizzle3] + offsets[swizzle3] 
        dataC = (c & 0x7f) * scales[swizzle4] + offsets[swizzle4] 

        dataD = math.sqrt((1 - (dataA * dataA + dataB * dataB + dataC * dataC))) 
    
        if (a & 0x80) != 0:
            dataD = -dataD

        quat = [None]*4
        quat[swizzle2] = dataA
        quat[swizzle3] = dataB
        quat[swizzle4] = dataC
        quat[swizzle1] = dataD

        return quat


    def GetQuterions(self):

        ScaleTable = [
                        1.4142135, 0.70710677, 0.35355338, 0.35355338,
                        0.35355338, 0.17677669, 0.17677669, 0.17677669,
                        -1.4142135, -0.70710677, -0.35355338, -0.35355338,
                        -0.35355338, -0.17677669, -0.17677669, -0.17677669]


        OffsetTable = [
                        -0.70710677, -0.35355338, -0.53033006, -0.17677669,
                        0.17677669, -0.17677669, -0.088388346, 0.0,
                        0.70710677, 0.35355338, 0.53033006, 0.17677669,
                        -0.17677669, 0.17677669, 0.088388346, -0.0]

        knots = self.Knots()

        #now we create quats
        scaleTable = [None]*4
        offsetTable = [None]*4
        selector = self.ScaleOffsetTableEntries

        scaleTable[0] = ScaleTable[(selector >> 0) & 0x0F] * 0.0078740157 
        scaleTable[1] = ScaleTable[(selector >> 4) & 0x0F] * 0.0078740157 
        scaleTable[2] = ScaleTable[(selector >> 8) & 0x0F] * 0.0078740157 
        scaleTable[3] = ScaleTable[(selector >> 12) & 0x0F] * 0.0078740157 

        offsetTable[0] =  OffsetTable[(selector >> 0) & 0x0F] 
        offsetTable[1] =  OffsetTable[(selector >> 4) & 0x0F] 
        offsetTable[2] =  OffsetTable[(selector >> 8) & 0x0F] 
        offsetTable[3] =  OffsetTable[(selector >> 12) & 0x0F] 

        numKnots = self.NumKnots()
        quaterions = []
        for i in range(numKnots):          
            quat = self.CreateQuat(self.KnotsControls[numKnots + i * 3 + 0],self.KnotsControls[numKnots + i * 3 + 1],self.KnotsControls[numKnots + i * 3 + 2],scaleTable,offsetTable)
            quaterions.append(quat) #w,x,y,z

        return quaterions,knots # knots is the time


class d3k16uc16u: # type 10

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//4
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetTranslation(self):
        numKnots = self.NumKnots()
        vectors = []
        for i in range(numKnots):
             v = [None]*3
             v[0] = self.KnotsControls[numKnots + i * 3 + 0] * self.ControlScales[0] + self.ControlOffsets[0]
             v[1] = self.KnotsControls[numKnots + i * 3 + 1] * self.ControlScales[1] + self.ControlOffsets[1]
             v[2] = self.KnotsControls[numKnots + i * 3 + 2] * self.ControlScales[2] + self.ControlOffsets[2]
             vectors.append(v)
         

        return vectors, self.Knots()


class d3k8uc8u: # type 11

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//4
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetTranslation(self):
        numKnots = self.NumKnots()
        vectors = []
        for i in range(numKnots):
             v = [None]*3
             v[0] = self.KnotsControls[numKnots + i * 3 + 0] * self.ControlScales[0] + self.ControlOffsets[0]
             v[1] = self.KnotsControls[numKnots + i * 3 + 1] * self.ControlScales[1] + self.ControlOffsets[1]
             v[2] = self.KnotsControls[numKnots + i * 3 + 2] * self.ControlScales[2] + self.ControlOffsets[2]
             vectors.append(v)
        
        return vectors, self.Knots()


class d9i1k16uc16u: # type 12

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScale = []
         self.ControlOffset = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//2
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetMartix(self):
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            mat[1] = 0
            mat[2] = 0
            mat[3] = 0
            mat[4] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            mat[5] = 0
            mat[6] = 0
            mat[7] = 0
            mat[8] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            matrix.append(mat)

        return matrix, self.Knots()


class d9i3k16uc16u: # type = 13
    
    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//4
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a


    def GetMartix(self): 
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i * 3 + 0] * self.ControlScales[0] + self.ControlOffsets[0]
            mat[1] = 0
            mat[2] = 0
            mat[3] = 0
            mat[4] = self.KnotsControls[numKnots + i * 3 + 1] * self.ControlScales[1] + self.ControlOffsets[1]
            mat[5] = 0
            mat[6] = 0
            mat[7] = 0
            mat[8] = self.KnotsControls[numKnots + i * 3 + 2] * self.ControlScales[2] + self.ControlOffsets[2]
            matrix.append(mat)

        return matrix, self.Knots()



class d9i1k8uc8u: # type = 14

    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScale = []
         self.ControlOffset = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//2
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetMartix(self): 
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            mat[1] = 0
            mat[2] = 0
            mat[3] = 0
            mat[4] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            mat[5] = 0
            mat[6] = 0
            mat[7] = 0
            mat[8] = self.KnotsControls[numKnots + i] * self.ControlScale + self.ControlOffset
            matrix.append(mat)

        return matrix, self.Knots()



class d9i3k8uc8u: # type = 15
    
    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []


    def NumKnots(self):
         return len(self.KnotsControls)//4
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a


    def GetMartix(self):
        numKnots = self.NumKnots()
        matrix = []
        for i in range(numKnots):
            mat = [None]*9
            mat[0] = self.KnotsControls[numKnots + i * 3 + 0] * self.ControlScales[0] + self.ControlOffsets[0]
            mat[1] = 0
            mat[2] = 0
            mat[3] = 0
            mat[4] = self.KnotsControls[numKnots + i * 3 + 1] * self.ControlScales[1] + self.ControlOffsets[1]
            mat[5] = 0
            mat[6] = 0
            mat[7] = 0
            mat[8] = self.KnotsControls[numKnots + i * 3 + 2] * self.ControlScales[2] + self.ControlOffsets[2]
            matrix.append(mat)

        return matrix, self.Knots()



class d3i1k32fc32f: # type = 16
    
    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.Padding = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//2
    
    def Knots(self):
        knots = []
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i])

        return knots

    def GetTranslation(self):
        numKnots = self.NumKnots()
        vectors = []
        for i in range(numKnots):
             v = [None]*3
             v[0] = self.KnotsControls[numKnots + i ] * self.ControlScales[0] + self.ControlOffsets[0]
             v[1] = self.KnotsControls[numKnots + i ] * self.ControlScales[1] + self.ControlOffsets[1]
             v[2] = self.KnotsControls[numKnots + i ] * self.ControlScales[2] + self.ControlOffsets[2]
             vectors.append(v)
        

        return vectors, self.Knots()


class d3i1k16uc16u: # type = 17
    
    def __init__(self):
         self.Format = 0
         self.Degree = 0
         self.OneOverKnotScaleTrunc = 0
         self.ControlScales = []
         self.ControlOffsets = []
         self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//2
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetTranslation(self):
        numKnots = self.NumKnots()
        vectors = []
        for i in range(numKnots):
             v = [None]*3
             v[0] = self.KnotsControls[numKnots + i ] * self.ControlScales[0] + self.ControlOffsets[0]
             v[1] = self.KnotsControls[numKnots + i ] * self.ControlScales[1] + self.ControlOffsets[1]
             v[2] = self.KnotsControls[numKnots + i ] * self.ControlScales[2] + self.ControlOffsets[2]
             vectors.append(v)
       

        return vectors, self.Knots()



class d3i1k8uc8u: #type = 18

    def __init__(self):
        self.Format = 0
        self.Degree = 0
        self.OneOverKnotScaleTrunc = 0
        self.ControlScales = []
        self.ControlOffsets = []
        self.KnotsControls = []

    def NumKnots(self):
         return len(self.KnotsControls)//2
    
    def Knots(self):
        knots = []
        scale = self.ConvertOneOverKnotScaleTrunc() 
        for i in range(self.NumKnots()):
            knots.append(self.KnotsControls[i]/scale)

        return knots

    def ConvertOneOverKnotScaleTrunc(self):
        a = self.OneOverKnotScaleTrunc << 16
        a = a.to_bytes(4,byteorder='little')
        a = struct.unpack("<f",a)[0]
        return a

    def GetTranslation(self):
        numKnots = self.NumKnots()
        vector = []
        for i in range(numKnots):
             v = [None]*3
             v[0] = self.KnotsControls[numKnots + i ] * self.ControlScales[0] + self.ControlOffsets[0]
             v[1] = self.KnotsControls[numKnots + i ] * self.ControlScales[1] + self.ControlOffsets[1]
             v[2] = self.KnotsControls[numKnots + i ] * self.ControlScales[2] + self.ControlOffsets[2]
             vector.append(v)

        return vector, self.Knots()



#general animation class
class Animations:

    def __init__(self):
        self.Duration = -1
        self.Name = ''
        self.Oversampling = -1
        self.TimeStep = -1
        self.Tracks = []



#used to upgarde old curve to new curves
class old_curev:
    def __init__(self):
        self.Name = ''
        self.PositionCurve = self.CurveData()
        self.OrientationCurve = self.CurveData()
        self.ScaleShearCurve = self.CurveData()

    class CurveData:
        def __init__(self):
            self.CurveData = []



#each track is an animation of a different model
class Transform_Tracks:
    def __init__(self):
        self.Name = ''
        self.PositionCurve = self.Curve()
        self.OrientationCurve = self.Curve()
        self.ScaleShearCurve = self.Curve()

    class Curve:
        def __init__(self): 
            self.Format = -1
            self.Degree = -1
            self.Knots = []
            self.Controls = [] 


class Tracks:

    def __init__(self): 
        self.InitialPlacement = []
        self.Name = []
        self.TransformTracks = [] 



class KeyFrame:
      def __init__(self):
          #should have 1 Translation, 1 rotation and 1 scale/shear per bone
          self.Name = ''
          self.time = 0.0
          self.Translation = -1
          self.Rotation = -1
          self.ScaleShear = -1
          self.Matrix = -1
          self.Orientation = -1 #1 means already fixed

      def __lt__(self, other):
        return self.time < other


#############################################################################
#Functions
#############################################################################

def CreateStructCurve(Format):
    if Format == 0:
         dummy = dakeyframes32f()
    if Format == 1:
         dummy = dak32fc32f()
    if Format == 2:
         dummy = daidentity()
    if Format == 3:
         dummy = daconstant32f()
    if Format == 4:
         dummy = d3constant32f()
    if Format == 5:
         dummy = d4constant32f()
    if Format == 6:
         dummy = dak16uc16u()
    if Format == 7:
         dummy = dak8uc8u()
    if Format == 8:
         dummy = d4nk16uc15u()
    if Format == 9:
         dummy =  d4nk8uc7u()
    if Format == 10:
         dummy = d3k16uc16u()
    if Format == 11:
         dummy =  d3k8uc8u()
    if Format == 12:
         dummy =  d9i1k16uc16u()
    if Format == 13:
         dummy =  d9i3k16uc16u()
    if Format == 14:
         dummy =  d9i1k8uc8u()
    if Format == 15:
         dummy =  d9i3k8uc8u()
    if Format == 16:
         dummy =  d3i1k32fc32f()
    if Format == 17:
         dummy =  d3i1k16uc16u()
    if Format == 18:
         dummy =  d3i1k8uc8u()
    
    return dummy


def get_CurveData_Format(Type):
    for obj in vars(Type.CurveData):
            if obj[:15] == 'CurveDataHeader':                       
                temp = getattr(Type.CurveData, obj)
                return temp.Degree, temp.Format

            if obj == 'Format':
                return Type.CurveData.Degree, Type.CurveData.Format


def Struct_To_List(Primary,secondery):
    temp  =[]
    if type(Primary) != list:
        if secondery:
            temp.append(getattr(Primary,secondery))
        else:
            temp.append(Primary)
            
    else:
        for p in Primary:
            if secondery:
                temp.append(getattr(p,secondery))
            else:
                temp.append(p)
    return temp



def GetCurveData(Format,Curve,input):

    #dakeyframes32f
    if Format == 0:
         Curve.Dimension = input.Dimension
         Curve.Controls = Struct_To_List(input.Controls,'Real32')

    #need to revisit - dak32fc32f
    if Format == 1:
        if input.Controls and hasattr(input.Controls[0], 'Real32'):         
             Curve.Padding = input.Padding
             Curve.Knots = Struct_To_List(input.Knots,'Real32')             
             Curve.Controls = Struct_To_List(input.Controls,'Real32')
        else:
             Curve.Padding = input.Padding
             Curve.Knots = input.Knots
             Curve.Controls = input.Controls
    #daidentity
    if Format == 2:
         Curve.Dimension = input.Dimension
    
    #daconstant32f
    if Format == 3:        
         Curve.Controls = Struct_To_List(input.Controls,'Real32')
         Curve.Padding = input.Padding 

    #d3constant32f
    if Format == 4:
        Curve.Controls = Struct_To_List(input.Controls, None)
        Curve.Padding = input.Padding

    #d4constant32f
    if Format == 5:
        Curve.Controls = Struct_To_List(input.Controls, None)
        Curve.Padding = input.Padding
        
    #dak16uc16u
    if Format == 6: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScaleOffsets =  Struct_To_List(input.ControlScaleOffsets, 'Real32')
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')

    #dak8uc8u
    if Format == 7: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScaleOffsets = Struct_To_List(input.ControlScaleOffsets, 'Real32')
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')

    #d4nk16uc15u
    if Format == 8:
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')
        Curve.OneOverKnotScale = input.OneOverKnotScale
        Curve.ScaleOffsetTableEntries = input.ScaleOffsetTableEntries

    #d4nk8uc7u
    if Format == 9:
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')
        Curve.OneOverKnotScale = input.OneOverKnotScale 
        Curve.ScaleOffsetTableEntries = input.ScaleOffsetTableEntries

    #d3k16uc16u
    if Format == 10:
        Curve.ControlOffsets = input.ControlOffsets
        Curve.ControlScales = input.ControlScales
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')        
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc

    #d3k8uc8u
    if Format == 11:
        Curve.ControlOffsets = input.ControlOffsets
        Curve.ControlScales = input.ControlScales
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        
    #d9i1k16uc16u
    if Format == 12: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScale = input.ControlScale
        Curve.ControlOffset = input.ControlOffset
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')
        
    #d9i3k16uc16u
    if Format == 13: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScales = input.ControlScales
        Curve.ControlOffsets = input.ControlOffsets
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')
        
    #d9i1k8uc8u
    if Format == 14: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScale = input.ControlScale
        Curve.ControlOffset = input.ControlOffset
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')
        
    #d9i3k8uc8u
    if Format == 15: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScales = input.ControlScales
        Curve.ControlOffsets = input.ControlOffsets
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')
        
    #d3i1k32fc32f
    if Format == 16: 
        Curve.Padding = input.Padding
        Curve.ControlScales = input.ControlScales
        Curve.ControlOffsets = input.ControlOffsets
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'Real32')

    #d3i1k16uc16u
    if Format == 17: 
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScales = input.ControlScales
        Curve.ControlOffsets = input.ControlOffsets
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt16')

    #d3i1k8uc8u
    if Format == 18:
        Curve.OneOverKnotScaleTrunc = input.OneOverKnotScaleTrunc
        Curve.ControlScales = input.ControlScales
        Curve.ControlOffsets = input.ControlOffsets
        Curve.KnotsControls = Struct_To_List(input.KnotsControls, 'UInt8')

    return Curve


def Convert(Degree,input):
    
    #Degree 0 - means there is no interpolation , so we would have only "static" animation
    if Degree == 0 and not input.Controls:
        dummy = daidentity()
        dummy.Degree = 0
        dummy.Format = 2
    #degree = 1 is linear interpolation , degree = 2 is quadratic interpolation, degree = 3 is cubic interpolation
    #if Degree == 1 or Degree == 2 or Degree == 3:
    else:
        dummy = dak32fc32f()
        dummy.Degree = Degree
        dummy.Format = 1
        dummy.Controls = input.Controls
        dummy.Knots = input.Knots

    return dummy


def UpgradeAnimation(TransformTrack):
        Dummy = old_curev()                         
        Dummy.OrientationCurve.CurveData = Convert(TransformTrack.OrientationCurve.Degree,TransformTrack.OrientationCurve)
        Dummy.PositionCurve.CurveData = Convert(TransformTrack.PositionCurve.Degree,TransformTrack.PositionCurve)        
        Dummy.ScaleShearCurve.CurveData = Convert(TransformTrack.ScaleShearCurve.Degree,TransformTrack.ScaleShearCurve)

        return Dummy


def GetRotation(OrientationCurve):
    Format = OrientationCurve.Format

    if Format == 0:
        if OrientationCurve.Dimension == 4:
            return OrientationCurve.GetQuterions() 
        if OrientationCurve.Dimension == 9:
            return OrientationCurve.GetMartix()

    if Format == 1:
         return OrientationCurve.GetQuterions() 


    if Format == 2:
         return OrientationCurve.GetQuterions()

    if Format == 3:
         length = len(OrientationCurve.Controls)
         if length == 4:
              return OrientationCurve.GetQuterions()
         if length == 9:
              return OrientationCurve.GetMartix()

    if Format == 4:
        print("bad rotation")
        # can only be vecor

    if Format == 5:
         return OrientationCurve.GetQuterions()

    if Format == 6:
        Dimension = len(OrientationCurve.ControlScaleOffsets)
        if Dimension == 9:
            return OrientationCurve.GetMartix()
        if Dimension == 4:
            return OrientationCurve.GetQuterions()

    if Format == 7:
         Dimension = len(OrientationCurve.ControlScaleOffsets)
         if Dimension == 9:
              return OrientationCurve.GetMartix()
         if Dimension == 4:
             return OrientationCurve.GetQuterions()

    if Format == 8:
         return OrientationCurve.GetQuterions()

    if Format == 9:
         return OrientationCurve.GetQuterions()

    if Format == 10:
        print("bad rotation")
        # can only be vecor

    if Format == 11:
        print("bad rotation")
        # can only be vecor

    if Format == 12:
        return OrientationCurve.GetMartix()

    if Format == 13:
        return OrientationCurve.GetMartix()

    if Format == 14:
        return OrientationCurve.GetMartix()

    if Format == 15:
         return OrientationCurve.GetMartix()

    if Format == 16:
        print("bad rotation")
        # can only be vecor

    if Format == 17:
        print("bad rotation")
        # can only be vecor

    if Format == 18:
        print("bad rotation")
        # can only be vecor


def GetTranslation(PositionCurve):
    Format = PositionCurve.Format

    if Format == 0:
        #Dimension should be 3
        return PositionCurve.GetTranslation()

    if Format == 1:
        #Dimension should be 3
        return PositionCurve.GetTranslation()

    if Format == 2:
        return PositionCurve.GetTranslation()

    if Format == 3:
        length = len(PositionCurve.Controls)
        if length == 3:
            return PositionCurve.GetTranslation()
        else:
            print("bad translation")

    if Format == 4:
        return PositionCurve.GetTranslation()

    if Format == 5:
        print("bad translation")
        # can only be quaterion

    if Format == 6:
        print("bad translation")
        # can only be quaterion or matrix

    if Format == 7:
        print("bad translation")
        # can only be quaterion or matrix

    if Format == 8:
        print("bad translation")
        # can only be quaterion

    if Format == 9:
        print("bad translation")
        # can only be quaterion

    if Format == 10:
        return PositionCurve.GetTranslation()

    if Format == 11:
        return PositionCurve.GetTranslation()

    if Format == 12:
        print("bad translation")
        # can only be matrix

    if Format == 13:
        print("bad translation")
        # can only be matrix

    if Format == 14:
        print("bad translation")
        # can only be matrix

    if Format == 15:
        print("bad translation")
        # can only be matrix

    if Format == 16:
        return PositionCurve.GetTranslation()

    if Format == 17:
        return PositionCurve.GetTranslation()

    if Format == 18:
        return PositionCurve.GetTranslation()


#get a given frame from data and fill in all the blanks using interpolation
def FrameInterpolate(Data):
    next = -1
    for i in range(len(Data) - 1):
        previous = Data[i]
        current = Data[i+1]
        
        if previous.Translation != -1 and current.Translation == -1:
            #now we find the next element that has a translation
            for j in range(i+1,len(Data)):
                if Data[j].Translation != -1:
                    next = Data[j]
                    break
            if next != -1 and next.Translation!= -1:                
                factor = (current.time - previous.time)/(next.time - previous.time)
                a = NoeVec3([previous.Translation[0], previous.Translation[1], previous.Translation[2]])
                b = NoeVec3([next.Translation[0], next.Translation[1], next.Translation[2]])
                c =  a.lerp(b,factor) 
                current.Translation = [c[0],c[1],c[2]]
            else:
                current.Translation = previous.Translation
       
        next = -1
        if previous.Rotation != -1 and current.Rotation == -1:
            #now we find the next element that has a Rotation
            for j in range(i+1,len(Data)):
                if Data[j].Rotation != -1:
                    next = Data[j]
                    break
            if next != -1 and next.Rotation!= -1:
                factor = (current.time - previous.time)/(next.time - previous.time)
                a = NoeQuat([previous.Rotation[0], previous.Rotation[1], previous.Rotation[2],previous.Rotation[3]]).normalize()
                b = NoeQuat([next.Rotation[0], next.Rotation[1], next.Rotation[2],next.Rotation[3]]).normalize()
                c = a.slerp(b,factor)
                current.Rotation = [c[0],c[1],c[2],c[3]]
            else:
                current.Rotation = previous.Rotation

        next = -1
        if previous.ScaleShear != -1 and current.ScaleShear == -1:
            for j in range(i+1,len(Data)):
                if Data[j].ScaleShear != -1:
                    next = Data[j]
                    break
            if next!= -1:
               current.ScaleShear = [None] * 9
               factor = (current.time - previous.time)/(next.time - previous.time)
               current.ScaleShear[0] = previous.ScaleShear[0] * (1 - factor) + next.ScaleShear[0] * factor
               current.ScaleShear[1] = previous.ScaleShear[1] * (1 - factor) + next.ScaleShear[1] * factor
               current.ScaleShear[2] = previous.ScaleShear[2] * (1 - factor) + next.ScaleShear[2] * factor
               current.ScaleShear[3] = previous.ScaleShear[3] * (1 - factor) + next.ScaleShear[3] * factor
               current.ScaleShear[4] = previous.ScaleShear[4] * (1 - factor) + next.ScaleShear[4] * factor
               current.ScaleShear[5] = previous.ScaleShear[5] * (1 - factor) + next.ScaleShear[5] * factor
               current.ScaleShear[6] = previous.ScaleShear[6] * (1 - factor) + next.ScaleShear[6] * factor
               current.ScaleShear[7] = previous.ScaleShear[7] * (1 - factor) + next.ScaleShear[7] * factor
               current.ScaleShear[8] = previous.ScaleShear[8] * (1 - factor) + next.ScaleShear[8] * factor
            else:
                current.ScaleShear = previous.ScaleShear
                
    return Data


def CreateKeyFrame(Data):

    FrameList = []

    for i,time in enumerate(Data.OrientationCurve.Knots):
        Dummy = KeyFrame()
        Dummy.time = time
        Dummy.Name = Data.Name
        Dummy.Rotation = Data.OrientationCurve.Controls[i]       
        FrameList.append(Dummy)

    for i,time in enumerate(Data.PositionCurve.Knots):

        index = bisect.bisect_left(FrameList, time)
        #if it alraedt exits
        if index < len(FrameList) and FrameList[index].time == time:
            FrameList[index].Translation = Data.PositionCurve.Controls[i]
        #if it dosent exist create new frame
        else:
            Dummy = KeyFrame()        
            Dummy.time = time
            Dummy.Name = Data.Name
            Dummy.Translation = Data.PositionCurve.Controls[i]
            FrameList.insert(index,Dummy)

    for i,time in enumerate(Data.ScaleShearCurve.Knots):

        index = bisect.bisect_left(FrameList, time)
        #if it alraedy exits
        if index < len(FrameList) and FrameList[index].time == time:
            FrameList[index].ScaleShear = Data.ScaleShearCurve.Controls[i]
        #if it dosent exist create new frame
        else:
            Dummy = KeyFrame()        
            Dummy.time = time
            Dummy.Name = Data.Name
            Dummy.ScaleShear = Data.ScaleShearCurve.Controls[i]
            FrameList.insert(index,Dummy)
            
    
    # sometimes 2 different frames are at time 0, so i delete the first and leave the second
    if len(FrameList) > 1 and FrameList[0].time == 0 and FrameList[1].time == 0:

        if FrameList[1].Translation == -1 and FrameList[0].Translation != -1:
            FrameList[1].Translation = FrameList[0].Translation
        if FrameList[1].Rotation == -1 and FrameList[0].Rotation != -1:
            FrameList[1].Rotation = FrameList[0].Rotation
        if FrameList[1].ScaleShear == -1 and FrameList[0].ScaleShear != -1:
            FrameList[1].ScaleShear = FrameList[0].ScaleShear

        del(FrameList[0])

    return FrameList


#function to equlize number of frames for all bones in order to allow flat list of bone_transform * frame
def EqualizeFrames(BoneFramelist,Duration,TimeStep):

    Max = -1
    
    for a in BoneFramelist:
          for b in a:
              if b.time > Max:
                  Max = b.time
    TimeList = []
    time = 0.0
    
    # all time steps are estimate need to add a calcualtion based on time step and duration - need more work here!!!
    if Max <= Duration + 0.5:
        while time < Max:
            TimeList.append(time)
            time += 1/30
    else:
        while time < Max:
            TimeList.append(time)
            time += 0.96
            
    if Max == 0:
        TimeList.append(time)
        
    BoneFrames = []
    frames = []
       
    # a represents all the frames of a specific bone
    Dummy = TimeList[:]
    for a in BoneFramelist:       
        Newlist = []
        for i,time in enumerate(Dummy):
            Index = bisect.bisect_left(a, time)
            #i would put it before index or insert in index
            if len(a) == 1:
                Newlist.append(a[0])
                continue
            if Index == 0 and time == 0.0:
                Newlist.append(a[Index])
            else:
                if Index <= len(a)-1:
                    factor = (time - a[Index-1].time)/(a[Index].time - a[Index-1].time)
                    Temp = FrameInterpolateSingle( a[Index-1],  a[Index], factor)
                    Temp.time = time
                    Newlist.append(Temp)
                else:
                    Temp = KeyFrame()
                    Temp.Name = Newlist[len(Newlist)-1].Name
                    Temp.Rotation = Newlist[len(Newlist)-1].Rotation                    
                    Temp.Translation = Newlist[len(Newlist)-1].Translation
                    Temp.ScaleShear = Newlist[len(Newlist)-1].ScaleShear
                    Temp.time = time
                    Newlist.append(Temp)                    
        
        ApplyShear(Newlist)
        frames.append(Newlist)

    
    return frames


def FrameInterpolateSingle(start, end, factor):
        new = KeyFrame()
        a = NoeQuat([start.Rotation[0], start.Rotation[1], start.Rotation[2],start.Rotation[3]])
        b = NoeQuat([end.Rotation[0], end.Rotation[1], end.Rotation[2],end.Rotation[3]]) 
        c = a.slerp(b,factor)
        new.Rotation = [c[0],c[1],c[2],c[3]]
        
        a = NoeVec3([start.Translation[0], start.Translation[1], start.Translation[2]])
        b = NoeVec3([end.Translation[0], end.Translation[1], end.Translation[2]])
        c =  a.lerp(b,factor) 
        new.Translation = [c[0],c[1],c[2]]
        
        new.ScaleShear = [0.0] * 9
        new.ScaleShear[0] = start.ScaleShear[0] * (1 - factor) + end.ScaleShear[0] * factor
        new.ScaleShear[1] = start.ScaleShear[1] * (1 - factor) + end.ScaleShear[1] * factor
        new.ScaleShear[2] = start.ScaleShear[2] * (1 - factor) + end.ScaleShear[2] * factor
        new.ScaleShear[3] = start.ScaleShear[3] * (1 - factor) + end.ScaleShear[3] * factor
        new.ScaleShear[4] = start.ScaleShear[4] * (1 - factor) + end.ScaleShear[4] * factor
        new.ScaleShear[5] = start.ScaleShear[5] * (1 - factor) + end.ScaleShear[5] * factor
        new.ScaleShear[6] = start.ScaleShear[6] * (1 - factor) + end.ScaleShear[6] * factor
        new.ScaleShear[7] = start.ScaleShear[7] * (1 - factor) + end.ScaleShear[7] * factor
        new.ScaleShear[8] = start.ScaleShear[8] * (1 - factor) + end.ScaleShear[8] * factor

        new.Name = start.Name

        return new


         
# apply scale/ shear matrix - to transform 
def ApplyShear(Frame):
    
    for i,m in enumerate(Frame):
        temp = Transform()
        temp.Quaterion = m.Rotation
        temp.Translation = m.Translation 
        temp.ScaleShear = m.ScaleShear
        
        if temp.ScaleShear == -1:
            temp.ScaleShear = [1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0]
        m.Matrix = ComposeLocalMatrix(temp)

    return Frame



class Transform:
    def __init__(self):
        self.flag = -1
        self.Translation = []
        self.Quaterion =  []
        self.ScaleShear = []

    def __eq__(self, other) : 
        if self.Translation == other.Translation and self.Quaterion == other.Quaterion and self.ScaleShear == other.ScaleShear and self.flag == other.flag: 
            return True
        else:
            return False

        
class Model:
    def __init__(self):
        self.Name = ''
        self.Bones = []
        self.Meshes = []
        self.AnimationInfo = []
        self.Trackgroups = []
        self.InitialPlacement = []
        self.OrinationInfo = []
        self.OrinationTransform = self.TransformData()

    class TransformData:
        def __init__(self): 
            self.Linear3x3 = []
            self.InverseLinear3x3 = []
            self.Affine3 = []
            
            
class Mesh:
    def __init__(self):        
        self.info = self.Info()
        self.mesh = self.Data()

    class Info:
        def __init__(self): 
            self.Mesh_Name = []
            self.Vertex_Count = []
            self.Face_Count = [] 
            self.Polygroups = []
            self.OrginalModelName = []
            self.TextureMaps = []
            self.Materials = []
            
    class Data:
        def __init__(self):
            self.Positions = []
            self.Normals = []
            self.TextureCoordinates = []
            self.TextureCoordinates2 = []
            self.Binormal = []
            self.Tangents = []
            self.Indices = []
            self.BoneWeights = []
            self.BoneIndices = []
            self.BoneBindings = []


class Skeleton:
    def __init__(self): 
        self.Name = ''
        self.BoneNames = []
        self.ParentIndex = []
        self.Transform = []
        self.InverseWorldTransform = []
        self.Bone_Count = 0 
 
#Main GR2 Reader Function        
def GR2Reader(data): 

    #=====================================================================
    #supporting Classes
    #=====================================================================
           
    class fd_packet_group():

        def __init__(self):
            self.PacketCount = 0
            self.FirstVertexIndex = 0
            self.BoneIndex = 0


    class fd_vertex_packet():

        def __init__(self):
            self.Position = []
            self.Normal = []
            self.Tangent = []
            self.TextureCoordinates0 = []


    class fd_vertex():

        def __init__(self):
            self.Position = []
            self.BoneWeights = []
            self.BoneIndices = []
            self.Normal = 0 
            self.Tangent = 0 
            self.TextureCoordinates0 = []


    class fd_pwngt34332_vertex():

        def __init__(self):
            self.Position = []
            self.BoneWeights = []
            self.BoneIndices = []
            self.Normal = []
            self.Tangent = []
            self.TextureCoordinates0 = []


    class fd_packet_group():

        def __init__(self):
            self.PacketCount = 0
            self.FirstVertexIndex = 0
            self.BoneIndex = 0


    class fd_header():

        def __init__(self):
            self.MagicValue = 0
            self.DataSize = 0
            self.CRC32 = 0
            self.TotalVertexCount = 0
            self.GroupCount = 0
            self.GroupSize = 0
            self.PacketCount = 0
            self.PacketSize = 0
            self.VertexCount = 0
            self.VertexSize = 0
            self.PacketVertexCount = 0 
            self.LargestBoneIndex  = 0 
            self.IncludeTangentVectors = 0

        
    class dummy_member:

        def __init__(self):
            self.random = []


    class MemberDefinition:
        
        def __init__(self):
            self.name = ""
            self.Type = 0
            self.definitionOffset = 0
            self.StringOffset = 0
            self.arraySize = 0
            self. extra = []
            self.unk = 0
            self.Data = []


    class StructDefinition:
        def __init__(self):
            self.Members = []
            self.type = None


    class Marshalling:
        def __init__(self):
            self.count = 0
            self.offset = 0
            self.target_section = 0
            self.target_offset = 0


    class Relocation:
        def __init__(self):
            self.offset = 0
            self.target_section = 0
            self.target_offset = 0


    class Section_Header:
        # always 44 bytes
         def __init__(self):
            self.compression = 0 # 0: no compression, 1: Oodle0, 2: Oodle1
            self.data_offset = 0 # From the start of the file
            self.data_size = 0  # In bytes
            self.decompressed_size = 0 # In bytes
            self.alignment = 0     # Seems always 4
            self.first16bit  = 0    # Stop0 for Oodle1
            self.first8bit    = 0    # Stop1 for Oodle1
            self.relocations_offset = 0
            self.relocations_count = 0
            self.marshallings_offset = 0
            self.marshallings_count = 0


    class Header:
        # always 32 bytes
         def __init__(self):
            self.magic = [] #16 bytes
            self.size = 0 # Size of the header
            self.format = 0  # Seems always 0
            self.reserved =[] #Seems always 0, 8 bytes



    class HeaderInfo:
        def __init__(self):
            # from here can be considered info part
            self.version = 0 # Always seems 6 or 7 depending on game
            self.file_size = 0
            self.crc32 = 0
            self.sections_offset = 0 #From 'version'
            self.sections_count = 0
            self.type_section = 0 #rootType
            self.type_offset = 0 #rootType
            self.root_section = 0 #rootNode
            self.root_offset = 0 #rootNode
            self.tag = 0
            self.extra = [] # 16 bytes, Always seems 0
            self.stringTableCrc = 0
            self.reserved1 = 0
            self.reserved2 = 0
            self.reserved3 = 0      


    class Refrence:
        def __init__(self):
            self.section = 0
            self.offset = 0
            self.type = None

    class root:
        def __init__(self):
            self.offset = 0
            self.type = None


    #===========================================================
    #Supporting functions
    #===========================================================
    
    FD_MAGIC  = 0xfdabcd01

    def FD_EXT_VEC(v, i):
        a = v.to_bytes(8,'little')[i]
        c = (a)/63.5
        return c -1
        
    def GetFDvertcies(f):

        StartLoc = f.tell()
        # Header
        header = fd_header()

        header.MagicValue = f.readUInt()
        header.DataSize = f.readUInt()
        header.CRC32 = f.readUInt()
        header.TotalVertexCount = f.readUInt()
        header.GroupCount = f.readUInt()
        header.GroupSize = f.readUInt()
        header.PacketCount = f.readUInt()
        header.PacketSize = f.readUInt()
        header.VertexCount = f.readUInt()
        header.VertexSize = f.readUInt()
        header.PacketVertexCount = f.readUInt()
        header.LargestBoneIndex  = f.readUInt()
        header.IncludeTangentVectors = f.readUInt()

        if header.MagicValue != FD_MAGIC:
            f.seek(StartLoc,0)
            return 0

        # Groups
        f.seek(StartLoc + 64,0)

        groups = []
        for gro in range(header.GroupCount):
            group = fd_packet_group()
            group.PacketCount = f.readUShort()
            group.FirstVertexIndex = f.readUShort()
            group.BoneIndex = f.readUInt()
            groups.append(group)


        # Packets
        f.seek(StartLoc + 64 + header.GroupSize,0)

        packets = []
        for pac in range(header.PacketCount):
            packet = fd_vertex_packet()

            for i in range(3):	
                vertex = struct.unpack("<ffff",f.readBytes(16))
                packet.Position.append(vertex)

            packet.Normal = struct.unpack("<IIII",f.readBytes(16))
            packet.Tangent = struct.unpack("<IIII",f.readBytes(16))

            for i in range(4):
                uv = struct.unpack("<ff",f.readBytes(8))
                packet.TextureCoordinates0.append(uv)

            packets.append(packet)


        # Vertcies
        f.seek(StartLoc + 64 + header.GroupSize + header.PacketSize,0)

        vertices = []
        for vert in range(header.VertexCount):
            vertice = fd_vertex()
            vertice.Position = struct.unpack("<fff",f.readBytes(12))
            vertice.BoneWeights = struct.unpack("<BB",f.readBytes(2))
            vertice.BoneIndices = struct.unpack("<BB",f.readBytes(2))
            vertice.Normal = struct.unpack("<I",f.readBytes(4))[0]
            vertice.Tangent = struct.unpack("<I",f.readBytes(4))[0]
            vertice.TextureCoordinates0 = struct.unpack("<ff",f.readBytes(8))
            vertices.append(vertice)


        DestVertices = [] 
        p = 0
        for g in range(header.GroupCount):
            for h in range(groups[g].PacketCount):
                for v in range(4):
                    verts = fd_pwngt34332_vertex()
                
                    vx = packets[p].Position[0][v]
                    vy = packets[p].Position[1][v]
                    vz = packets[p].Position[2][v]

                    Tex0 = packets[p].TextureCoordinates0[v][0]
                    Tex1 = packets[p].TextureCoordinates0[v][1]

                    Normal0 = FD_EXT_VEC(packets[p].Normal[v], 0)
                    Normal1 = FD_EXT_VEC(packets[p].Normal[v], 1)
                    Normal2 = FD_EXT_VEC(packets[p].Normal[v], 2)

                    Tangent0 = FD_EXT_VEC(packets[p].Tangent[v], 0)
                    Tangent1 = FD_EXT_VEC(packets[p].Tangent[v], 1)
                    Tangent2 = FD_EXT_VEC(packets[p].Tangent[v], 2)

                    verts.Position = vx,vy,vz
                    verts.BoneWeights = 255,0,0,0
                    verts.BoneIndices = groups[g].BoneIndex,0,0,0
                    verts.TextureCoordinates0 = Tex0,Tex1
                    verts.Normal = Normal0,Normal1,Normal2
                    verts.Tangent = Tangent0,Tangent1,Tangent2

                    DestVertices.append(verts)

                p += 1


        for v in range(header.VertexCount):
        
            vertice = fd_pwngt34332_vertex()

            vx = vertices[v].Position[0]
            vy = vertices[v].Position[1]
            vz = vertices[v].Position[2]

            Normal0 = FD_EXT_VEC(vertices[v].Normal, 0)
            Normal1 = FD_EXT_VEC(vertices[v].Normal, 1)
            Normal2 = FD_EXT_VEC(vertices[v].Normal, 2)

            Tangent0 = FD_EXT_VEC(vertices[v].Tangent, 0)
            Tangent1 = FD_EXT_VEC(vertices[v].Tangent, 1)
            Tangent2 = FD_EXT_VEC(vertices[v].Tangent, 2)

            Tex0 = vertices[v].TextureCoordinates0[0]
            Tex1 = vertices[v].TextureCoordinates0[1]

            vertice.Position = vx,vy,vz
            #vertice.Position = list((vx,vy,vz))
            vertice.BoneWeights = vertices[v].BoneWeights[0],vertices[v].BoneWeights[1],0,0
            vertice.BoneIndices = vertices[v].BoneIndices[0],vertices[v].BoneIndices[1],0,0
            vertice.Normal = Normal0,Normal1,Normal2
            vertice.Tangent = Tangent0,Tangent1,Tangent2
            vertice.TextureCoordinates0 = Tex0,Tex1

            DestVertices.append(vertice)

        return DestVertices

    
    def NormilzeUV(TextureCoordinates):
        Ucoordinates = []
        Vcoordinates = []
        
        for i in range(len(TextureCoordinates)):
              Ucoordinates.append(TextureCoordinates[i][0])
              Vcoordinates.append(TextureCoordinates[i][1])
        
        max_U = max(Ucoordinates)
        max_V = max(Vcoordinates)
        
        if max_U != 0 and max_V != 0:
            for i in range(len(TextureCoordinates)):
                #Normlize Values
                TextureCoordinates[i][0] /= max_U           
                TextureCoordinates[i][1] /= max_V
                
                #Round to 4 digits
                TextureCoordinates[i][0] = round(TextureCoordinates[i][0], 4)
                TextureCoordinates[i][1] = round(TextureCoordinates[i][1], 4)  
                
        return TextureCoordinates
                                
                                
    def OrinationTransformCalc(Models,UnitsPerMeter_old,ArtToolInfo):
    
        Affine3 = [0,0,0]
        Origin =  [0,0,0]

        #Current Coordinate System  - right, up, back, origin
        Orination = [[],[],[],[]]
        Orination[0] = ArtToolInfo.RightVector
        Orination[1] = ArtToolInfo.UpVector
        Orination[2] = ArtToolInfo.BackVector
        Orination[3] = ArtToolInfo.Origin
        Models.OrinationInfo = Orination
        

        #Calculate Linear3x3 - needed to transform to new system
        Current_Orination = NoeMat43([NoeVec3(Orination[0]),NoeVec3(Orination[1]),NoeVec3(Orination[2]),NoeVec3(Orination[3])])
        Wanted_Orination = NoeMat43([NoeVec3(RightVector),NoeVec3(UpVector),NoeVec3(BackVector),NoeVec3(Origin)])
        Linear3x3 = (Wanted_Orination.inverse())* (Current_Orination)

        for i in range(3):
            for j in range(3):
                Linear3x3[i][j] = Linear3x3[i][j] * UnitsPerMeter_new/UnitsPerMeter_old

        InverseLinear3x3 = Linear3x3.inverse()

        Affine3[0] = Origin[0] - Orination[3][0]
        Affine3[1] = Origin[1] - Orination[3][1]
        Affine3[2] = Origin[2] - Orination[3][2]
        
        Models.OrinationTransform.Linear3x3 = Linear3x3
        Models.OrinationTransform.InverseLinear3x3 = InverseLinear3x3
        Models.OrinationTransform.Affine3 = Affine3
        
        return Linear3x3,InverseLinear3x3,Affine3
        
        
    def extractData(StructHeaders):
        All_Models = []
        
        #create this here incase file dosent have model data
        Models = Model()

        for model in StructHeaders.Models:

            Models = Model()
            
            if StructHeaders.ArtToolInfo and TRANSFORM_FILE:
            
                UnitsPerMeter_old = StructHeaders.ArtToolInfo.UnitsPerMeter
                Linear3x3,InverseLinear3x3,Affine3 = OrinationTransformCalc(Models,UnitsPerMeter_old,StructHeaders.ArtToolInfo)
                ReWindIndcies = 0

                #if a rotation matrix is a reflection (determinant is -1) it reverses "handedness" if determinant is 1 then "handedness" is preserved.
                det = CalculateDeterminant (Linear3x3)
                if det < 0:
                    ReWindIndcies = 1
                
            Models.Name = model.Name
            if hasattr( model, "InitialPlacement"):
                Models.InitialPlacement =  model.InitialPlacement
                
            #if we have mesh Data
            if hasattr(model, "MeshBindings") and model.MeshBindings and model.MeshBindings[0].Mesh.PrimaryVertexData:

                Num_Meshes = len(model.MeshBindings)
                mesh_array = []

                for i in range(Num_Meshes):
                    m = Mesh()
                    Positions = []
                    Normals = []
                    TextureCoordinates = []
                    TextureCoordinates2 = []
                    BoneIndices = []
                    BoneWeights = []
                    Binormal = []
                    Tangent = []
                    Indices = []
                    BoneBindings = []
                    PolyGroups = []
                    Meshtextures = []
                    VertexScale = 1
                    
                    vertcies_Data = model.MeshBindings[i].Mesh.PrimaryVertexData.Vertices
                    tri_Data = model.MeshBindings[i].Mesh.PrimaryTopology
                    Num_Vertices = len(vertcies_Data)
                       
                    # added PrevTriFirst check due to multiple materials to same group would cause appeneding same polys multiple times and out of bounds error
                    tri_Data.Groups.sort(key=lambda x: x.TriFirst)
                    
                    PrevTriFirst = -1
                    for Polygroup in tri_Data.Groups:
                        if Polygroup.TriFirst != PrevTriFirst:
                            PolyGroups.append(Polygroup.TriCount)
                        PrevTriFirst = Polygroup.TriFirst
                        
                    #for diablo 2 resurrected
                    if hasattr(model.MeshBindings[i].Mesh, "ExtendedData") and model.MeshBindings[i].Mesh.ExtendedData and hasattr(model.MeshBindings[i].Mesh.ExtendedData, "VertexScale"):
                        VertexScale = model.MeshBindings[i].Mesh.ExtendedData.VertexScale  
                        
                        #when mesh has multiple materials
                        if type(model.MeshBindings[i].Mesh.MaterialBindings) == list:
                            for matCount,mat in enumerate(model.MeshBindings[i].Mesh.MaterialBindings):
                                   if matCount < len(PolyGroups):
                                       temp_mat = []
                                       for Tex in mat.Material.Maps:
                                           if Tex.Map.Texture.FromFileName.lower() not in Meshtextures:
                                                temp_mat.append(Tex.Map.Texture.FromFileName.lower())
                                       Meshtextures.append(temp_mat)

                        else:
                            #for mesh with single material
                            temp_mat = []
                            for Tex in model.MeshBindings[i].Mesh.MaterialBindings.Material.Maps:
                                    if Tex.Map.Texture.FromFileName.lower() not in Meshtextures:
                                            temp_mat.append(Tex.Map.Texture.FromFileName.lower())
                            Meshtextures.append(temp_mat)

                        m.info.TextureMaps = Meshtextures
                
                    if  tri_Data.Indices16:
                        Num_Indices = len(model.MeshBindings[i].Mesh.PrimaryTopology.Indices16)
                    if  tri_Data.Indices:
                        Num_Indices = len(model.MeshBindings[i].Mesh.PrimaryTopology.Indices)

                    Bone_binds_data = model.MeshBindings[i].Mesh.BoneBindings
                    for j in range(len(Bone_binds_data)):
                            BoneBindings.append(Bone_binds_data[j].BoneName)
                            
                    for j in range(Num_Vertices):
                
                        if hasattr(vertcies_Data[j], "Position"):
                            if VertexScale != 1:
                                vertcies_Data[j].Position = [x * VertexScale for x in vertcies_Data[j].Position]
                            Positions.append(vertcies_Data[j].Position)
                        if hasattr(vertcies_Data[j], "Normal"):
                            Normals.append(vertcies_Data[j].Normal)
                        if hasattr(vertcies_Data[j], "TextureCoordinates0"):
                            TextureCoordinates.append(vertcies_Data[j].TextureCoordinates0)
                        if hasattr(vertcies_Data[j], "TextureCoord0"):
                            TextureCoordinates.append(vertcies_Data[j].TextureCoord0)
                        if hasattr(vertcies_Data[j], "TextureCoordinates1"):
                            TextureCoordinates2.append(vertcies_Data[j].TextureCoordinates1)
                        if hasattr(vertcies_Data[j], "TextureCoord1"):
                            TextureCoordinates2.append(vertcies_Data[j].TextureCoord1)
                        if hasattr(vertcies_Data[j], "BoneIndices"):
                            BoneIndices.append(vertcies_Data[j].BoneIndices)
                        if hasattr(vertcies_Data[j], "BoneWeights"):
                            BoneWeights.append(vertcies_Data[j].BoneWeights)
                        if hasattr(vertcies_Data[j], "Binormal"):
                            Binormal.append(vertcies_Data[j].Binormal)
                        if hasattr(vertcies_Data[j], "Tangent"):
                            Tangent.append(vertcies_Data[j].Tangent)
                            
                        if hasattr(vertcies_Data[j], "QTangent"):
                        
                            x = vertcies_Data[j].QTangent[0] * 0.000030518044
                            y = vertcies_Data[j].QTangent[1] * 0.000030518044
                            z = vertcies_Data[j].QTangent[2] * 0.000030518044
                            w = vertcies_Data[j].QTangent[3] * 0.000030518044
                            Matrix = NoeQuat((x,y,z,w)).toMat43()
                            if w < 0:
                                Matrix[2] *= -1
                                
                            Normals.append(Matrix[2])
                            Tangent.append(Matrix[1])
                            Binormal.append(Matrix[0])

                    for idx in range(Num_Indices):
                        if  tri_Data.Indices16:
                            Indices.append(tri_Data.Indices16[idx].Int16)
                        if  tri_Data.Indices:
                            Indices.append(tri_Data.Indices[idx].Int32)
                                                           
                    
                    if GAME_TAG_ESO  and hasattr(vertcies_Data[0], 'TextureCoord0'):
                        print("i am in a ESO static")
                    
                        if TextureCoordinates:
                            TextureCoordinates = NormilzeUV(TextureCoordinates)
                                
                        if TextureCoordinates2:
                            TextureCoordinates2 = NormilzeUV(TextureCoordinates2)

                    if GAME_TAG_SH5 or GAME_TAG_GRANNY:
                    
                        Material_SH = []
                        Materials = model.MeshBindings[i].Mesh.MaterialBindings
                        
                        if Materials:
                            if type(Materials)!= list:
                                DummyMat = []
                                DummyMat.append(Materials)
                                Materials = DummyMat
                        
                            for Mat in Materials:
                                if Mat.Material:
                                    new_Mat = SH_Material()
                                    Material_SH.append(new_Mat)
                                    new_Mat.Name = Mat.Material.Name
                                    if  Mat.Material.Maps:
                                        get_SH_maps(Mat.Material.Maps,new_Mat)
                        
                            m.info.Materials = Material_SH
                            
                            
                    if StructHeaders.ArtToolInfo and TRANSFORM_FILE:
                        TranformVertcies (Linear3x3,Positions,Normals,Tangent,Binormal,Affine3,UnitsPerMeter_old,UnitsPerMeter_new)
                        if ReWindIndcies:
                            RewindIndcies (Indices)
                            
                    m.mesh.Positions = Positions
                    m.mesh.Normals = Normals
                    m.mesh.TextureCoordinates = TextureCoordinates
                    m.mesh.TextureCoordinates2 = TextureCoordinates2
                    m.mesh.BoneIndices = BoneIndices
                    m.mesh.BoneWeights = BoneWeights
                    m.mesh.Tangents = Tangent
                    m.mesh.Binormal = Binormal
                    m.mesh.Indices = Indices
                    m.mesh.BoneBindings = BoneBindings
                    m.info.Polygroups = PolyGroups
                    m.info.Mesh_Name = model.MeshBindings[i].Mesh.Name
                    m.info.Vertex_Count = len(Positions)
                    m.info.Face_Count = len(Indices)//3
                    mesh_array.append(m)

                Models.Meshes = mesh_array
                
            #if we have skeleton data
            if hasattr(model, "Skeleton") and model.Skeleton:

                for skel in model.Skeleton:
                    s = Skeleton()
                    Bone_names = []
                    InverseWorldTransform = []
                    LODError = []
                    ParentIndex = []
                    Loacl_Transform = []
                   
                    for Bone in skel.Bones:
                        if hasattr(Bone, "Name"):
                            Bone_names.append(Bone.Name)
                        if hasattr(Bone, "InverseWorldTransform"):
                            InverseWorldTransform.append( Bone.InverseWorldTransform)
                        if hasattr(Bone, "LODError"):
                            LODError.append(Bone.LODError)
                        if hasattr(Bone, "ParentIndex"):
                            ParentIndex.append(Bone.ParentIndex)
                        if hasattr(Bone, "Transform"):
                            Loacl_Transform.append(Bone.Transform)

                    if StructHeaders.ArtToolInfo and TRANSFORM_FILE:
                        TransformSkeleton(Linear3x3,InverseLinear3x3, Affine3,InverseWorldTransform,UnitsPerMeter_old,UnitsPerMeter_new)
                        TransformSkeletonLocal(Linear3x3,InverseLinear3x3,Loacl_Transform,Affine3)
                    
                    s.InverseWorldTransform = InverseWorldTransform
                    s.Transform = Loacl_Transform
                    s.ParentIndex = ParentIndex
                    s.BoneNames = Bone_names
                    s.Bone_Count = len(s.ParentIndex)
                    s.Name = skel.Name
                    if len(model.Skeleton) == 1:
                        Models.Bones = s
            
            All_Models.append(Models)
            
            
        if StructHeaders.Animations:
            
            if StructHeaders.ArtToolInfo and TRANSFORM_FILE and ANIMATION_MODE == 1:
                UnitsPerMeter_old = StructHeaders.ArtToolInfo.UnitsPerMeter
                Linear3x3,InverseLinear3x3,Affine3 = OrinationTransformCalc(Models,UnitsPerMeter_old,StructHeaders.ArtToolInfo)
                
            if  ANIMATION_TRACK > 0 and ANIMATION_TRACK <= len(StructHeaders.Animations):
                anim = StructHeaders.Animations[ANIMATION_TRACK - 1]
            else:
                anim = StructHeaders.Animations[0]
            
            a =  Animations()
            a.Duration = anim.Duration
            a.Name = anim.Name
            #old format didnt have oversampeling
            if hasattr(anim, "Oversampling"):
                a.Oversampling = anim.Oversampling
            else:
                a.Oversampling = 0
            a.TimeStep = anim.TimeStep
            a.Tracks = anim.TrackGroups

            TrackGroups = []

            for track in a.Tracks:

                Tr = Tracks()
                Tr.Name = track.Name
                Tr.InitialPlacement = track.InitialPlacement
                TransformTracks = []
                
                if hasattr(track, "TransformTracks"):
                
                    for Transform_Track in track.TransformTracks:
                        t = Transform_Tracks()
                        t.Name = Transform_Track.Name
                        
                        #check if old curve format - need to upgrade
                        if not hasattr(Transform_Track.OrientationCurve, 'CurveData'):
                                Transform_Track = UpgradeAnimation(Transform_Track)                    
                        
                        DegreeO,FormatO = get_CurveData_Format(Transform_Track.OrientationCurve)
                        t.OrientationCurve = CreateStructCurve(FormatO)
                        t.OrientationCurve = GetCurveData(FormatO,t.OrientationCurve,Transform_Track.OrientationCurve.CurveData)
                        t.OrientationCurve.Degree = DegreeO
                        t.OrientationCurve.Format = FormatO

                        DegreeP,FormatP = get_CurveData_Format(Transform_Track.PositionCurve)
                        t.PositionCurve = CreateStructCurve(FormatP)
                        t.PositionCurve = GetCurveData(FormatP,t.PositionCurve,Transform_Track.PositionCurve.CurveData)
                        t.PositionCurve.Degree = DegreeP
                        t.PositionCurve.Format = FormatP

                        DegreeS,FormatS = get_CurveData_Format(Transform_Track.ScaleShearCurve)
                        t.ScaleShearCurve = CreateStructCurve(FormatS)
                        t.ScaleShearCurve = GetCurveData(FormatS,t.ScaleShearCurve,Transform_Track.ScaleShearCurve.CurveData)
                        t.ScaleShearCurve.Degree = DegreeS
                        t.ScaleShearCurve.Format = FormatS

                        TransformTracks.append(t)
                    
                    Tr.TransformTracks = TransformTracks
                    TrackGroups.append(Tr)

            a.Tracks = TrackGroups
            
            
            #for test only -- put track group inside its model class
            for model in All_Models:
                for track in a.Tracks:
                    if track.Name == model.Name:
                        model.Trackgroups = track
                        model.AnimationInfo = a
                        
                # if no track with coressponding name to model found insert last track to model
                if not model.Trackgroups:
                    model.Trackgroups = track
                    model.AnimationInfo = a
            
            #if we didnt have any model data, use empty struct taht was created and insert all animation data
            if not All_Models:
                Models.AnimationInfo = a
                transformTracks = []
                for j in range(len(TrackGroups)):
                    transformTracks += TrackGroups[j].TransformTracks
                TrackGroups[0].TransformTracks = transformTracks
                Models.Trackgroups = TrackGroups[0]
                All_Models.append(Models)
                        
        if StructHeaders.Models:
        
            if  MERGE_SCENE == 1:
                MergeModels = Model()
                tempMesh = []
                tempBones = []
                tempTracks = []
                tempInitialPlacement = []
                
                MergeModels.OrinationTransform.Linear3x3 = All_Models[0].OrinationTransform.Linear3x3
                MergeModels.OrinationTransform.InverseLinear3x3 = All_Models[0].OrinationTransform.InverseLinear3x3
                MergeModels.OrinationTransform.Affine3 = All_Models[0].OrinationTransform.Affine3
                
                if StructHeaders.Animations:
                    MergeModels.AnimationInfo = a
                    for trackIndex,track in enumerate(a.Tracks):
                        tempTracks += track.TransformTracks
                        tempInitialPlacement.append(track.InitialPlacement)
                            
                    MergeModels.Trackgroups = Tracks()
                    MergeModels.Trackgroups.TransformTracks = tempTracks
                    MergeModels.Trackgroups.InitialPlacement = tempInitialPlacement
                    
                for model in All_Models:
                    for i in range(len(model.Meshes)):
                        tempMesh.append(model.Meshes[i])
                        
                    if model.Bones:
                        tempBones.append(model.Bones)
                        MergeModels.Bones = tempBones
                        
                MergeModels.Meshes = tempMesh
                
                MergeModels.Name = 'merged_scene'
                models = []
                models.append(MergeModels)
                return models
            
        return All_Models


    def memberSize(type,member):
        if type == 1:
            size = 0
            originalPos = stream.tell()
            Members = readSubStructV2(stream,Format, member.definitionOffset)
            stream.seek(originalPos,0)
            for member in Members:
                size += memberSize(member.Type,member)
            return size

        if type == 11 or type == 12 or type == 13 or type == 14:
            return 1

        if type == 15 or type == 16 or type == 17 or type == 18 or type == 21: # i added type 21
            return 2

        if type == 2 or type == 8 or type == 19 or type == 20 or type == 10:
            return 4

        if type == 3 or type == 4 or type == 5:
            return 8
        
        if type == 7:
            return 12

        if type == 9:
            return 68



    def MarshallingSize(type):
        if type == 1 or type == 2 or type == 5 or type == 22: #inline,refrence,variantRefrence,empty refrence
            return 0

        if type == 11 or type == 12 or type == 13 or type == 14: #Int8,Bin8,Uint8,Norm8
            return 1

        if type == 15 or type == 16 or type == 17 or type == 18 or type == 21: # int16,Bin16,Uint16,Norm16,Real16
            return 2

        if type == 8 or type == 9 or type == 10 or type == 19 or type == 20 or type == 3 or type == 7 or type == 4:# transform,string.real32,Uint32,Int32,Refrencetoarray,arrayofreffrencees,refrence to varient array
            return 4


    def MixedMarshal(stream,count, Defenition_Offset):
        originalPos = stream.tell()
        Members = readSubStructV2(stream,Format, Defenition_Offset)
        stream.seek(originalPos,0)
        for j in range(count):
            for member in Members:
                size = memberSize(member.Type,member)
                marshalSize = MarshallingSize(member.Type)

                if member.Type == 1: 
                    if member.arraySize == 0:
                        count = 1
                    else:
                        count = member.arraySize

                    MixedMarshal(stream,count, member.definitionOffset)

                elif marshalSize > 1:
                    byte_array = bytearray(stream.readBytes(size))
                    #byte_array.reverse() - not good
                    for i in range(size//marshalSize):
                        for off in range(marshalSize//2):
                            temp = byte_array[i * marshalSize + off]
                            byte_array[i * marshalSize + off] = byte_array[i * marshalSize + marshalSize - 1 - off]
                            byte_array[i * marshalSize + marshalSize - 1 - off] = temp
                    stream.seek(-size,1)
                    stream.writeBytes(byte_array)
                    stream.seek(-size,1)

                stream.seek(size,1)


    def readformat(stream, Format):
        if Format == "LittleEndian32" or Format == "BigEndian32":
            Data_offset = stream.readUInt()
        else:
            Data_offset = stream.readUInt64()

        return Data_offset


    def createStruct(Headers):
        m = dummy_member()
        for header in Headers:
            setattr(m, header.name, [])

        return m
    

    #Sets start location for each memebr
    def seekContoller(Members,MemberClass):
        global  Name_List
        Name_List = ['MeshBindings','Models','Bones','Skeleton', 'Skeletons','TrackGroups','Animations','TransformTracks', 'BoneBindings', 'Groups']
        originalPos = stream.tell()
        for member in Members:
            array = []
            array.append(member)   
            if member.name == 'Models' or member.name == 'Animations' or member.name == 'ArtToolInfo' or member.name == 'FileInfo':
                GetMemberData(stream,array,MemberClass)
            if Format == "LittleEndian32" or Format == "BigEndian32":
                originalPos += 4
            else:
                originalPos += 8
            if member.Type == 3 or member.Type == 4:
                originalPos += 4
            stream.seek(originalPos)

        return MemberClass
    
    
    #get data for member
    def GetMemberData(stream,members,ParentMember):
         
        for member in members:            
            
            if GAME_TAG_ESO and member.name == 'TextureCoord0' or GAME_TAG_ESO and member.name == 'TextureCoord1':
                    member.Type = 15
                    
            if member.arraySize > 0:
                temp = []
                
                ConvertToFloat = 1
                
                if (member.Type == 15  or member.Type == 17) and member.name == 'Position':
                    #convert short to float
                    ConvertToFloat = 1/32767.0
                
                if (member.Type == 11 or member.Type == 13) and (member.name == 'Normal' or member.name == 'Tangent'):
                    #convert Byte to float
                    ConvertToFloat = 1/127.0
                    
                for j in range(member.arraySize):
                    if member.Type == 19: 
                        temp.append(stream.readInt())
                    if member.Type == 10:    
                        temp.append(stream.readFloat())
                    if member.Type == 11 or member.Type == 13:
                        temp.append(stream.readByte() * ConvertToFloat)
                    if member.Type == 12 or member.Type == 14:
                        temp.append(stream.readUByte())
                    if member.Type == 15 or member.Type == 17:
                        temp.append(stream.readShort() * ConvertToFloat)
                    if member.Type == 16 or member.Type == 18:
                        temp.append(stream.readUShort())
                    if member.Type == 20: 
                        temp.append(stream.readUInt())
                    if member.Type == 21: 
                        temp.append(stream.readHalfFloat())

                #member.Data.append(temp)
                setattr(ParentMember, member.name, temp)
                continue     

            #None
            if member.Type == 0: 
                member.Data = []
                continue

            # inline
            if member.Type == 1: 
                originalPos = stream.tell()
                AllSubHeaders = readSubStructV2(stream,Format, member.definitionOffset)
                StrucM = createStruct(AllSubHeaders)
                stream.seek(originalPos,0) 
                GetMemberData(stream, AllSubHeaders,StrucM)
                if member.Data != None:              
                    setattr(ParentMember, member.name, StrucM)
                continue          

            # refrence
            if member.Type == 2:
                Data_offset = readformat(stream, Format)
                if Data_offset == 0:
                    continue
                ContinueOffset = stream.tell()
                if member.definitionOffset:
                    AllSubHeaders = readSubStructV2(stream,Format, member.definitionOffset)
                    StrucM = createStruct(AllSubHeaders)
                   
                stream.seek(Data_offset,0)
                GetMemberData(stream, AllSubHeaders,StrucM)

                if member.name in Name_List:
                    array = []
                    array.append(StrucM)
                    setattr(ParentMember, member.name, array)               
                else:         
                    setattr(ParentMember, member.name, StrucM)
                    
                stream.seek(ContinueOffset,0)        
                continue

            #Type == 3 ReferenceToArray
            #Type == 7 ReferenceToVariantArray
            if member.Type == 3 or member.Type == 7:
                if  member.Type == 7:
                    member.definitionOffset = readformat(stream, Format) 
                size = stream.readUInt()
                offset = readformat(stream, Format)                     
                if size == 0 or offset == 0:              
                    continue
                ContinueOffset = stream.tell()           
                if member.definitionOffset:            
                    AllSubHeaders = readSubStructV2(stream,Format,  member.definitionOffset)
                    StrucM = createStruct(AllSubHeaders)

                stream.seek(offset,0)
                
                if 'FD' == AllSubHeaders[0].name:
                    array = GetFDvertcies(stream)
                    size = len(array)
                
                else:
                    
                    if member.name == 'PixelBytes' or member.name == 'Pixels':
                        array = []
                        for j in range(size):               
                            GetMemberData(stream,AllSubHeaders,StrucM)
                            array.append(StrucM.UInt8)
                        
                    elif size > 1 or member.name in Name_List:
                        array = []
                        for  tempP in range(size):
                            array.append(StrucM.__class__())
                        for j in range(size):               
                            GetMemberData(stream,AllSubHeaders,array[j])
                    else:
                        for j in range(size):               
                            GetMemberData(stream,AllSubHeaders,StrucM)

                if size > 1 or member.name in Name_List:
                    setattr(ParentMember, member.name, array)               
                else:               
                    setattr(ParentMember, member.name, StrucM)
                  
                stream.seek(ContinueOffset,0)     
                continue

            #ArrayOfReferences
            if member.Type == 4:
                RefrenceOffset = []
                size = stream.readUInt()
                Data_offset = readformat(stream, Format)
                if size == 0 or Data_offset == 0:
                    continue
                ContinueOffset = stream.tell()         
                if member.definitionOffset: 
                    AllSubHeaders = readSubStructV2(stream,Format, member.definitionOffset)
                    StrucM = createStruct(AllSubHeaders)

                stream.seek(Data_offset,0)
                for j in range(size):               
                    RefrenceOffset.append(readformat(stream, Format)) 

                if size > 1 or member.name in Name_List:
                    array = []
                    for  tempP in range(size):
                        array.append(StrucM.__class__())
                    for j in range(size):
                        stream.seek(RefrenceOffset[j],0)
                        GetMemberData(stream, AllSubHeaders,array[j])
                else:
                    for j in range(size):
                        stream.seek(RefrenceOffset[j],0)
                        GetMemberData(stream, AllSubHeaders,StrucM)
            
                if size > 1 or member.name in Name_List:
                    setattr(ParentMember, member.name, array)              
                else:
                    setattr(ParentMember, member.name, StrucM)
                    
                stream.seek(ContinueOffset,0)       
                continue

            #extended data - currently i ignore it - "VariantReference"
            if member.Type == 5: 
                defnition_offset = readformat(stream, Format)
                Data_offset = readformat(stream, Format)
                #if member.name == 'ExtendedData':
                #    member.Data = []
                #    continue
                if defnition_offset == 0:
                    continue
                ContinueOffset = stream.tell()          
                AllSubHeaders = readSubStructV2(stream,Format, defnition_offset)
                StrucM = createStruct(AllSubHeaders)
                stream.seek(Data_offset,0)
                GetMemberData(stream, AllSubHeaders,StrucM)
                setattr(ParentMember, member.name, StrucM)
                stream.seek(ContinueOffset,0)          
                continue

            #string
            if member.Type == 8:
                StringOffset = readformat(stream, Format)
                ContinueOffset = stream.tell()
                stream.seek(StringOffset,0)
                if ParentMember:
                    setattr(ParentMember, member.name, readString(stream))
                stream.seek(ContinueOffset,0)             
                continue

            # Transform Data
            if member.Type == 9:
                node = Transform()
                node.flag = stream.readUInt()

                # Translation
                for cord in range(3):
                    node.Translation.append(stream.readFloat())

                #Quaterion x,y,z,w
                for cord in range(4):
                    node.Quaterion.append(stream.readFloat()) 

                for cord in range(9):
                    node.ScaleShear.append(stream.readFloat())
                
                setattr(ParentMember, member.name, node)
                continue

            #float
            if member.Type == 10:    
                setattr(ParentMember, member.name, stream.readFloat())
                continue

            # 1 byte - char
            if member.Type == 11 or member.Type == 13: 
                setattr(ParentMember, member.name, stream.readByte())
                continue

            # 1 byte - unsigned char
            if member.Type == 12 or member.Type == 14: 
                setattr(ParentMember, member.name, stream.readUByte())
                continue

            #short/Int16
            if member.Type == 15 or member.Type == 17: 
                setattr(ParentMember, member.name, stream.readShort())
                continue

            #unsigned short
            if member.Type == 16 or member.Type == 18: 
                setattr(ParentMember, member.name, stream.readUShort())
                continue

            # int
            if member.Type == 19: 
                setattr(ParentMember, member.name, stream.readInt())
                continue

            # unsigned int
            if member.Type == 20: 
                setattr(ParentMember, member.name, stream.readUInt())
                continue

            # half float
            if member.Type == 21: 
                setattr(ParentMember, member.name, stream.readHalfFloat())
                continue

            #type == 22 ==> empty refrence
               

    def readString(stream):
        string = []
        while(1):
            bytes = stream.readBytes(1)
            if bytes == b'\x00':
                break
            string += bytes
        try:
            string = bytearray(string).decode(encoding='UTF-8')
        except:
            string = "Unk"

        return string


    #check format of file 64/32 bit
    def FormatType(String):
    
        #civ6 format - mixed format - 64 bit for header mix marshelling etc.., 32 bit for navigating ArtTool etc..
        LittleEndian32v4 = [0x5B, 0x6C, 0xD6, 0xD2, 0x3C, 0x46, 0x8B, 0xD6, 0x83, 0xC2, 0xAA, 0x99, 0x3F, 0xE1, 0x76, 0x52]
    
        #Magic value used for version 6 little-endian 32-bit Granny files
        LittleEndian32v3 = [0xB8, 0x67, 0xB0, 0xCA, 0xF8, 0x6D, 0xB1, 0x0F, 0x84, 0x72, 0x8C, 0x7E, 0x5E, 0x19, 0x00, 0x1E]
        
        #Magic value used for version 7 little-endian 32-bit Granny files
        LittleEndian32v1 = [0x29, 0xDE, 0x6C, 0xC0, 0xBA, 0xA4, 0x53, 0x2B, 0x25, 0xF5, 0xB7, 0xA5, 0xF6, 0x66, 0xE2, 0xEE]

        #Magic value used for version 7 little-endian 32-bit Granny files
        LittleEndian32v2 = [0x29, 0x75, 0x31, 0x82, 0xBA, 0x02, 0x11, 0x77, 0x25, 0x3A, 0x60, 0x2F, 0xF6, 0x6A, 0x8C, 0x2E]
        
        #Magic value used for version 7 big-endian 32-bit Granny files
        BigEndian32v1 = [0x0E, 0x11, 0x95, 0xB5, 0x6A, 0xA5, 0xB5, 0x4B, 0xEB, 0x28, 0x28, 0x50, 0x25, 0x78, 0xB3, 0x04]

        #Magic value used for version 7 big-endian 32-bit Granny files
        BigEndian32v2 = [0x0E, 0x74, 0xA2, 0x0A, 0x6A, 0xEB, 0xEB, 0x64, 0xEB, 0x4E, 0x1E, 0xAB, 0x25, 0x91, 0xDB, 0x8F]

        #Magic value used for version 7 little-endian 64-bit Granny files
        LittleEndian64v1 = [0xE5, 0x9B, 0x49, 0x5E, 0x6F, 0x63, 0x1F, 0x14, 0x1E, 0x13, 0xEB, 0xA9, 0x90, 0xBE, 0xED, 0xC4]

        #Magic value used for version 7 little-endian 64-bit Granny files
        LittleEndian64v2 = [0xE5, 0x2F, 0x4A, 0xE1, 0x6F, 0xC2, 0x8A, 0xEE, 0x1E, 0xD2, 0xB4, 0x4C, 0x90, 0xD7, 0x55, 0xAF]

        #Magic value used for version 7 big-endian 64-bit Granny files
        BigEndian64v1 = [0x31, 0x95, 0xD4, 0xE3, 0x20, 0xDC, 0x4F, 0x62, 0xCC, 0x36, 0xD0, 0x3A, 0xB1, 0x82, 0xFF, 0x89]

        #Magic value used for version 7 big-endian 64-bit Granny files
        BigEndian64v2 = [0x31, 0xC2, 0x4E, 0x7C, 0x20, 0x40, 0xA3, 0x25, 0xCC, 0xE1, 0xC2, 0x7A, 0xB1, 0x32, 0x49, 0xF3]

        #Magic value used for version 7 big-endian 64-bit Granny files - console 
        BigEndian32v3 = [0xB5, 0x95, 0x11, 0x0E, 0x4B, 0xB5, 0xA5, 0x6A, 0x50, 0x28, 0x28, 0xEB, 0x04, 0xB3, 0x78, 0x25]
        
        if String == bytearray(LittleEndian32v1) or String == bytearray(LittleEndian32v2) or String == bytearray(LittleEndian32v3) or String == bytearray(LittleEndian32v4):
            return "LittleEndian32" 

        elif String == bytearray(LittleEndian64v1) or String == bytearray(LittleEndian64v2):
            return "LittleEndian64" 
                    
        elif String == bytearray(BigEndian64v1) or String == bytearray(BigEndian64v2):
            return "BigEndian64" 
            
        elif String == bytearray(BigEndian32v1) or String == bytearray(BigEndian32v2) or String == bytearray(BigEndian32v3):
            return "BigEndian32"   
        
        else:        
            return None

    # 64 bit - member is 44 bytes
    # 32 bit - member is 32 bytes
    def readStruct(stream, offset, Format):
    
        stream.seek(offset,0)
        member = MemberDefinition()
        member.Type = stream.readUInt()
        if member.Type == 0:
            return member
        member.StringOffset = readformat(stream, Format)
        member.definitionOffset = readformat(stream, Format)
        member.arraySize = stream.readUInt()
        for i in range(3):
            member.extra.append(stream.readUInt())
        if Format == 'LittleEndian32' or Format == 'BigEndian32':
            stream.readBytes(4)
        else:
            stream.readBytes(8)
        Position = stream.tell()
        stream.seek(member.StringOffset,0)
        if  member.Type != 0:
            member.name =  readString(stream)
        stream.seek(Position,0)
        return member



    def readSubStructV2(stream, MagicType, definitionOffset): 
        AllSubHeaders = []
        while(True):
            SubHeader = readStruct(stream,definitionOffset, MagicType) 
            if SubHeader.Type != 0:
                AllSubHeaders.append(SubHeader)
                definitionOffset = stream.tell()
            else:
                return AllSubHeaders


    def InsertBytes(Data,offset,byte_array):
        for i in range(4):
            Data[offset+i] = byte_array[i]
            
    
    def read_relocations(index,file,AllDecompressedData,section_offsets):
        Relocations = []
                
        for i in range(section.relocations_count):
            dummy = Relocation()
            dummy.offset = file.readUInt()
            dummy.target_section = file.readUInt()
            dummy.target_offset = file.readUInt()
            Relocations.append(dummy)

        
        # apply relocations
        for relocation in Relocations:
            byte_array = bytearray()
            source_offset  = section_offsets[index] + relocation.offset;
            target_offset = section_offsets[relocation.target_section] + relocation.target_offset;
            if Format == "BigEndian64" or Format == "BigEndian32":
                byte_array = target_offset.to_bytes(4,'big')
            else:
                byte_array = target_offset.to_bytes(4,'little')
            InsertBytes(AllDecompressedData,source_offset,byte_array)


    def ReadMarsheling(section,file,section_offsets,num_section):
        for i in range(section.marshallings_count):
            count = file.readUInt()
            OffsetInSection = file.readUInt()
            Marshel_Section = file.readUInt()
            Marshel_Offset = file.readUInt() # target offset
            Defenition_Offset = section_offsets[info.type_section] + Marshel_Offset
            stream.seek(section_offsets[num_section] + OffsetInSection,0) # 3 is a place holder
            MixedMarshal(stream,count, Defenition_Offset)


    def GR2decompress(DecompressedData,ComperesedData,decompressed_size,compressed_size,section):
        
        reverseBytes  = 0
        
        if Format == "BigEndian64" or Format == "BigEndian32":
            reverseBytes  = 1
        
        DLL_PATH = rapi.getDirForFilePath(noesis.getMainPath())
        
        if SysEnvironment == 32:
            lib = ctypes.WinDLL(DLL_PATH + "granny2.dll")
            GrannyDecompressData = lib['_GrannyDecompressData@32']
            beginDecompressProc = lib['_GrannyBeginFileDecompression@24']
            decompressProc = lib['_GrannyDecompressIncremental@12']
            endDecompressProc = lib['_GrannyEndFileDecompression@4']
        else:
            lib = ctypes.WinDLL(DLL_PATH + "granny2_x64.dll")
            GrannyDecompressData = lib['GrannyDecompressData']
            beginDecompressProc = lib['GrannyBeginFileDecompression']
            decompressProc = lib['GrannyDecompressIncremental']
            endDecompressProc = lib['GrannyEndFileDecompression']
            
        if section.compression == 1 or section.compression == 2:
            #Declare function argument types - as far as i know not necessary 
            GrannyDecompressData.argtypes = (ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_void_p,ctypes.c_int32,ctypes.c_int32,ctypes.c_int32,ctypes.c_void_p)
            #declare result type
            GrannyDecompressData.restype = ctypes.c_int32
            #send all aguments to function and get result in DecompressedData
            value = GrannyDecompressData(section.compression,reverseBytes,compressed_size,ComperesedData,section.first16bit,section.first8bit,decompressed_size,DecompressedData)

        if section.compression == 3 or section.compression == 4:
            try:

                WorkMemSize = 0x4000
                WorkMemBuffer = ctypes.cast(ctypes.create_string_buffer(WorkMemSize),ctypes.POINTER(ctypes.c_char))
                beginDecompressProc.restype = ctypes.POINTER(ctypes.c_void_p)
                state = beginDecompressProc(section.compression,reverseBytes,decompressed_size,DecompressedData,WorkMemSize, WorkMemBuffer)
                Position = 0
                while(Position < compressed_size):
                    chunkSize = min(compressed_size - Position, 0x2000)
                    incrementOk = decompressProc(state, chunkSize, ComperesedData[Position:])
                    if (incrementOk != 1):
                        print("Failed to decompress")
                    Position += chunkSize        
                ok = endDecompressProc(state)
                if ok != 1:
                    print("Failed to decompress")
                    
            except:
                noesis.messagePrompt('BitKnit Compression is not support by Current Granny2.dll Version , Please replace with a newer Version')
                return 0
            
        return DecompressedData
    
    def Calculate_CRC(hsize,fsize):
        f.seek(hsize,0)
        crc = f.readBytes(fsize - hsize)
        return zlib.crc32(crc)
        
    #===================================================================
    #start
    #===================================================================

  
    f = NoeBitStream(data, NOE_LITTLEENDIAN)

    #====================================================================
    # header
    #====================================================================

    header = Header()
    info = HeaderInfo()
      
    Format = FormatType(f.readBytes(16))
   
    if Format == None:
        print("Format not supported")
        return 0
    
    if Format == "BigEndian64" or Format == "BigEndian32":
        f = NoeBitStream(data, NOE_BIGENDIAN)
    
    f.seek(0,0)
    for i in range(4):
        header.magic.append(f.readUInt())

    header.size = f.readUInt()
    header.format = f.readUInt()


    for i in range(2):
        header.reserved.append(f.readUInt())

    info.version = f.readUInt()
    info.file_size = f.readUInt()
    crc32Location = f.tell()
    info.crc32 = f.readUInt()
    info.sections_offset = f.readUInt() #From 'version'
    info.sections_count = f.readUInt()
    info.type_section = f.readUInt()
    info.type_offset = f.readUInt()
    info.root_section = f.readUInt()
    info.root_offset = f.readUInt()
    info.tag = f.readUInt()

    for i in range(4):
        info.extra.append(f.readUInt())

    if info.version == 7:
        info.stringTableCrc = f.readUInt()
        info.reserved1 = f.readUInt()
        info.reserved2 = f.readUInt()
        info.reserved3 = f.readUInt() 

    if CRC_CORRECTION:
        HeaderSize = f.tell()
        FileSize = f.getSize()
        crc32 = Calculate_CRC(HeaderSize ,FileSize)
        f.seek(HeaderSize,0)
        if crc32 != info.crc32:
            PATH = rapi.getDirForFilePath(rapi.getInputName())
            baseNameExt = rapi.getLocalFileName(rapi.getInputName())
            
            with open(PATH + baseNameExt, 'r+b') as fp:
                fp.seek(crc32Location,0)
                crc32 = struct.pack('I',crc32)
                fp.write(crc32)
            
        f.seek(HeaderSize,0)
    #====================================================================
    # section header
    #====================================================================

    SectionHeaders = []

    for i in range(info.sections_count):
        dummy = Section_Header()
        dummy.compression = f.readUInt()    
        dummy.data_offset = f.readUInt() #offset In File
        dummy.data_size = f.readUInt() # compressed Size
        dummy.decompressed_size = f.readUInt()
        dummy.alignment = f.readUInt()
        dummy.first16bit = f.readUInt()
        dummy.first8bit  = f.readUInt()
        dummy.relocations_offset = f.readUInt()
        dummy.relocations_count = f.readUInt()
        dummy.marshallings_offset = f.readUInt() #mixedMarshallingDataOffset
        dummy.marshallings_count = f.readUInt()
        SectionHeaders.append(dummy)

    section_offsets = []
    size = 0

    section_offsets.append(0)
    for offset in SectionHeaders:
        section_offsets.append(offset.decompressed_size + size) 
        size += offset.decompressed_size

    section_offsets = section_offsets[:info.sections_count] # need to test make sure is always correct
    Section_totalSize = section_offsets[len(section_offsets) - 1]

    #====================================================================
    #Data Decompressing and manipulation
    #====================================================================


    AllDecompressedData = []
    # read sections and decompress using granny2.dll
    for section in SectionHeaders:
        if section.compression == 0:
            f.seek(section.data_offset,0)
            AllDecompressedData += f.readBytes(section.data_size)
            continue
        if section.data_size == 0:
            continue
        ComperesedData = []
        
        DLL_PATH = rapi.getDirForFilePath(noesis.getMainPath())
        
        if SysEnvironment == 32:
            if not rapi.checkFileExists(DLL_PATH + 'granny2.dll'):
                noesis.messagePrompt('Cant locate granny2.dll, please place it in Noesis Main Folder')
                return 0
        else:
            if not rapi.checkFileExists(DLL_PATH + 'granny2_x64.dll'):
                noesis.messagePrompt('Cant locate granny2_x64.dll, please place it in Noesis Main Folder')
                return 0
          
        #create bytes object of wanted size 
        DecompressedData = bytes(section.decompressed_size)
        f.seek(section.data_offset,0)
        ComperesedData = f.readBytes(section.data_size)
        TempAllDecompressedData = GR2decompress(DecompressedData,ComperesedData,section.decompressed_size,section.data_size,section)

        if not TempAllDecompressedData:
            return 0
            
        AllDecompressedData += TempAllDecompressedData
     
    
    #read and apply relocation
    index = 0
    for section in SectionHeaders:
        if section.relocations_count == 0:
            index += 1
            continue

        f.seek(section.relocations_offset,0)
                
        if section.compression == 3 or section.compression == 4:

            ComperesedData = []
            DecompressedData = []
            DecompressedSection = []

            DecompressedData = bytes(section.relocations_count * 12)
            CompressedSize = f.readUInt()
            ComperesedData = f.readBytes(CompressedSize)
            DecompressedSection = GR2decompress(DecompressedData,ComperesedData,section.relocations_count * 12,CompressedSize,section)
            
            SectionStream = NoeBitStream(bytes(DecompressedSection), NOE_LITTLEENDIAN)
            
            if Format == "BigEndian64" or Format == "BigEndian32":
                SectionStream = NoeBitStream(bytes(DecompressedSection), NOE_BIGENDIAN)

            SectionStream.seek(0,0)

            read_relocations(index,SectionStream,AllDecompressedData,section_offsets)

        else:
            read_relocations(index,f,AllDecompressedData,section_offsets)

        index += 1
    
    stream = NoeBitStream(bytearray(AllDecompressedData), NOE_LITTLEENDIAN)
    
    if Format == "BigEndian64" or Format == "BigEndian32":
        stream = NoeBitStream(bytearray(AllDecompressedData), NOE_BIGENDIAN)
        
    stream.seek(0,0)
       
    #need marsheling only if on none litle endian system????
    if Format != "LittleEndian32" and Format != "LittleEndian64":  
        num_section = 0   
        # marsheling
        for section in SectionHeaders: 
            if section.marshallings_count <= 0:
                num_section += 1
                continue
            f.seek(section.marshallings_offset,0)
            if section.compression == 3 or section.compression == 4:

                ComperesedData = []
                DecompressedData = []
                DecompressedSection = []
                DecompressedData = bytes(section.marshallings_count * 16)
                CompressedSize = f.readUInt()
                ComperesedData = f.readBytes(CompressedSize)
                DecompressedSection = GR2decompress(DecompressedData,ComperesedData,section.marshallings_count * 16,CompressedSize,section)
                
                SectionStream = NoeBitStream(bytes(DecompressedSection), NOE_LITTLEENDIAN)
                
                if Format == "BigEndian64" or Format == "BigEndian32":
                    SectionStream = NoeBitStream(bytes(DecompressedSection), NOE_BIGENDIAN)
                    
                SectionStream.seek(0,0)

                ReadMarsheling(section,SectionStream,section_offsets,num_section)

            else:
                ReadMarsheling(section,f,section_offsets,num_section)
    
    #=================================================================================
    # reading all data - verts,faces,bones etc...
    #=================================================================================
       
    root_section = root()
    root_section.offset = section_offsets[info.type_section] + info.type_offset
    stream.seek(info.root_offset,0)
    
    stream = NoeBitStream(bytearray(AllDecompressedData), NOE_LITTLEENDIAN)
    
    if Format == "BigEndian64" or Format == "BigEndian32":
        stream = NoeBitStream(bytearray(AllDecompressedData), NOE_BIGENDIAN)
        
    Members = []

    Position = root_section.offset
    while(True):
        member  = readStruct(stream,Position ,Format)
        if member.Type == 0:
            break
        Members.append(member)
        Position = stream.tell()
   
    #after applying marsheling changes    
    stream.seek(0,0)   
    
    MemberClass = createStruct(Members)
    
    #civ6 game fix
    if hasattr(MemberClass, "FxsModels"):
       civ6m = dummy_member()
       for mn in vars(MemberClass):
           if mn[:3] == 'Fxs':
                setattr(civ6m, mn[3:], [])
       MemberClass = civ6m
         
    for member in Members:
        if member.name[:3] == 'Fxs':
            member.name = member.name[3:]
                       
    #main gr2 TreeWalker function 
    StructHeaders = seekContoller(Members,MemberClass)
    
    Models = extractData(StructHeaders)

    return Models
 

def ComposeLocalMatrix(Transform):
        if hasattr(Transform, "Quaterion"):
            q = Transform.Quaterion
        if hasattr(Transform, "Rotation"):
            q = Transform.Rotation
        t = Transform.Translation
        s = Transform.ScaleShear

        R = NoeQuat((q[0],q[1],q[2],q[3])).toMat43()
        S = NoeMat43([(s[0], s[1], s[2]), (s[3], s[4], s[5]), (s[6], s[7], s[8]), (0, 0, 0)])
        C = R*S
        C = C.transpose()
        
        C[3][0] = t[0]
        C[3][1] = t[1]
        C[3][2] = t[2]

        return C

#check if 2 arrays are equal within a certin threshhold
def WithinTol(a,b,Threshold):
    if len(a) != len(b):
        return False

    for i in range(len(a)):
        if abs(a[i]-b[i]) > Threshold:
            return False

    return True


#Removes Duplicate bones, renames bones with the same name that are different and updates mesh bone bindings 
def RemoveDuplicateBones(Combined):
    
    Threshold = 0.000000001
    
    #replace parent index with parent bone name
    for i in range(len(Combined.Bones)):
        for j in range(len(Combined.Bones[i].BoneNames)):
            if Combined.Bones[i].ParentIndex[j] != -1:
                Combined.Bones[i].ParentIndex[j] = Combined.Bones[i].BoneNames[Combined.Bones[i].ParentIndex[j]]

    #always compare to combine0
    for k in range(len(Combined.Bones)-1):
    
        BoneNames1 = Combined.Bones[0].BoneNames
        BoneNames2 = Combined.Bones[k+1].BoneNames

        for j,name in enumerate(BoneNames1):
            if name in BoneNames2:
                index = BoneNames2.index(name)
            else:
                continue
                 
            #delete any bones with identical name, dont check any other parameter - for models that we know share the same skeleton
            if SMART_DETECTION == 1:

                del(Combined.Bones[k+1].BoneNames[index])
                del(Combined.Bones[k+1].InverseWorldTransform[index])
                del(Combined.Bones[k+1].ParentIndex[index])
                del(Combined.Bones[k+1].Transform[index])
           
            #if bone name , parent index and Inverse transform (within Threshold) are identical then delete from second skeleton
            if SMART_DETECTION == 2 and WithinTol(Combined.Bones[0].InverseWorldTransform[j],Combined.Bones[k+1].InverseWorldTransform[index],Threshold):#  and Combined.Bones[0].ParentIndex[j] == Combined.Bones[1].ParentIndex[index]:             
                
                del(Combined.Bones[k+1].BoneNames[index])
                del(Combined.Bones[k+1].InverseWorldTransform[index])
                del(Combined.Bones[k+1].ParentIndex[index])
                del(Combined.Bones[k+1].Transform[index])
                
        #when i renamed bone in this loop parent index name was also changed which caused identical bones to seem unequal

        #bone name exists but they are not identical - then rename bone and rename parent bone name
        for j,name in enumerate(BoneNames1):
            if name in BoneNames2:
                index = BoneNames2.index(name)
            else:
                continue

            oldname = name
            name = name + '_' + str(j)
            BoneNames2[index] = name
            Combined.Bones[k+1].BoneNames[index] = name

            indices = [Bidx for Bidx, x in enumerate(Combined.Bones[k+1].ParentIndex) if x == oldname]
            for Bidx in indices:
                Combined.Bones[k+1].ParentIndex[Bidx] = name

            #need to go thru meshbindings of relevent meshes and update names
            for mesh in Combined.Meshes:
                if Combined.Bones[k+1].Meshes and mesh.info.Mesh_Name in Combined.Bones[k+1].Meshes:
                    #then update
                    if oldname in mesh.mesh.BoneBindings:
                        index = mesh.mesh.BoneBindings.index(oldname)
                        mesh.mesh.BoneBindings[index] = name

        Combined.Bones[1].Bone_Count = len(Combined.Bones[1].BoneNames)

        if Combined.Bones[1].Bone_Count > 0:
            Combined.Bones[0].BoneNames += Combined.Bones[k+1].BoneNames
            Combined.Bones[0].InverseWorldTransform += Combined.Bones[k+1].InverseWorldTransform
            Combined.Bones[0].ParentIndex += Combined.Bones[k+1].ParentIndex
            Combined.Bones[0].Transform += Combined.Bones[k+1].Transform
            Combined.Bones[0].Bone_Count = len(Combined.Bones[0].BoneNames)

   #replace parent bone names with parent bone index
    for j in range(len(Combined.Bones[0].BoneNames)):
        if Combined.Bones[0].ParentIndex[j] != -1:
            Combined.Bones[0].ParentIndex[j] = Combined.Bones[0].BoneNames.index(Combined.Bones[0].ParentIndex[j])
    
    Combined.Bones = Combined.Bones[0]
    Combined.Bones.Name = 'Merged'
    Combined.Bones.InitialPlacement = -1


    return Combined
    
#for multiple files    
def MergeSkeletons2(model):    
    Combined = RemoveDuplicateBones(model)        
    return  SkeletonLocale(Combined)
 

#not used
def FindColsestBone(BoneBindings, ref, candidates, BoneIndices, Positions,BoneNames,BoneWeights,Count,NewIndex):

    candidates.sort(key=lambda x: (x[0] - ref[0]) ** 2 + (x[1] - ref[1]) ** 2 + (x[2] - ref[2]) ** 2)
    for cad in candidates:     
        for c in range(4):
            bname = BoneBindings[BoneIndices[Positions.index(cad)][c]]
            if bname in (BoneNames):                                        
                NewIndex[0] = BoneIndices[Positions.index(cad)][c]
                BoneWeights[Count][0] = 1
                BoneWeights[Count][1] = BoneWeights[Count][2] = BoneWeights[Count][3] = 0
                break
        if max(BoneWeights[Count]) == 1:
            break           
    
    return NewIndex

#for multiple models in same file 
def MergeSkeletons(model):
    bones = []   
    
    for i in range(len(model.Bones)):
        temp = Model()
        temp.Bones = model.Bones[i] 

        #if MULTIFILE == 0:
        #    temp.InitialPlacement = model.Bones[i].InitialPlacement
        ModelBones = SkeletonLocale(temp)
        if i == 0:
            bones += ModelBones
            continue
        else:
           correction_num = len(bones)
           for bo in ModelBones:
                bo.index += correction_num
                if bo.parentIndex == -1:
                    continue
                else:
                    bo.parentIndex += correction_num
           bones += ModelBones

    return  bones

     
def UpdatePostMerge(model):
    tempNames = []
    tempTransforma = []
    for bones in model.Bones:
        tempNames += bones.BoneNames
        tempTransforma += bones.Transform
    #everything except for bone names is of no use anymore so i dont bother to upadte
    model.Bones = Skeleton()
    model.Bones.BoneNames = tempNames
    model.Bones.Bone_Count = len(tempNames)
    model.Bones.InverseWorldTransform = -1
    model.Bones.Name = 'Merged'
    model.Bones.Transform = tempTransforma
    #already applied InitialPlacement to bones so no need to keep
    model.Bones.InitialPlacement = -1
    
    return model.Bones


#load skeleton data - using inverse world transform
def SkeletonWorld(model):

    bones = []
    
    Name = model.Bones.Name
    BoneCount = model.Bones.Bone_Count
    BoneNameArray = model.Bones.BoneNames
    ParentIndex = model.Bones.ParentIndex
    boneMats = model.Bones.InverseWorldTransform
    if model.InitialPlacement:
        Initial_location = ComposeLocalMatrix(model.InitialPlacement)
        
    if BoneCount > 1:
        for i in range(BoneCount):
            Matrix =  boneMats[i]
            boneMat = NoeMat44( [(Matrix[0],Matrix[1],Matrix[2],Matrix[3]), 
                                 (Matrix[4],Matrix[5],Matrix[6],Matrix[7]), 
                                 (Matrix[8],Matrix[9],Matrix[10],Matrix[11]), 
                                 (Matrix[12],Matrix[13],Matrix[14],Matrix[15])] )
                                 
            boneMat = boneMat.toMat43().inverse()

            bones.append( NoeBone(i, BoneNameArray[i], boneMat, None, ParentIndex[i]) )
                                           
        
    return bones
    
    
#load skeleton data - using local transform
def SkeletonLocale(model):   
    bones = []        
    Name = model.Bones.Name
    BoneCount = model.Bones.Bone_Count
    BoneNameArray = model.Bones.BoneNames
    ParentIndex = model.Bones.ParentIndex
    Transformations  = model.Bones.Transform
    if model.InitialPlacement:
        Initial_location = ComposeLocalMatrix(model.InitialPlacement)

    for i in range(BoneCount):
       boneMat = ComposeLocalMatrix(Transformations[i])
       
       #use world inverse matrix only for root bone to fix orintation (already includs Initial_location)
       if i == 0:
            Matrix = model.Bones.InverseWorldTransform[0]

            boneMat = NoeMat44( [(Matrix[0],Matrix[1],Matrix[2],Matrix[3]), 
                     (Matrix[4],Matrix[5],Matrix[6],Matrix[7]), 
                     (Matrix[8],Matrix[9],Matrix[10],Matrix[11]), 
                     (Matrix[12],Matrix[13],Matrix[14],Matrix[15])]).toMat43().inverse()

       bones.append( NoeBone(i, BoneNameArray[i], boneMat, None, ParentIndex[i]) )
    

    # Converting local matrix to world space
    for i in range(0, BoneCount):
        j = bones[i].parentIndex
        if j != -1:        
            bones[i].setMatrix(bones[i].getMatrix().__mul__(bones[j].getMatrix()))
      
    return bones  
    
def CreateBytesData(Positions,Normals,UV,UV2,Tangents,BoneIndex,BoneWights,Faces):

    NewBytes = bytearray()
        
    for position in Positions:
        var = struct.pack('fff',position[0],position[1],position[2])
        NewBytes += var
         
    for Normal in Normals:
        var = struct.pack('fff',Normal[0],Normal[1],Normal[2])
        NewBytes += var
        
    for coord in UV:
        var = struct.pack('ff',coord[0],coord[1])
        NewBytes += var
      
    for coord in UV2:
        var = struct.pack('ff',coord[0],coord[1])
        NewBytes += var
        
    for Tangent in Tangents:
        var = struct.pack('ffff',Tangent[0],Tangent[1],Tangent[2],1.0)
        NewBytes += var
      
    for index in BoneIndex:
        var = struct.pack('IIII',index[0],index[1],index[2], index[3]) 
        NewBytes += var
           
    for BoneWight in BoneWights:
        var = struct.pack('ffff',BoneWight[0],BoneWight[1],BoneWight[2], BoneWight[3])
        NewBytes += var
     
    #for negative face index assuming type is Int16 - short
    lowest = min(Faces)
    if lowest < 0:
        tempbytes = bytearray()
        for tri in Faces:       
            var = struct.pack('h',tri)
            tempbytes += var
            
        In = NoeBitStream(tempbytes, NOE_LITTLEENDIAN) 
        In.seek(0,0)
        
        for i in range(len(Faces)):
            var = In.readUShort()
            var = struct.pack('I',var)
            NewBytes += var
    else:
        for tri in Faces:       
            var = struct.pack('I',tri)
            NewBytes += var
    
    return NewBytes

    
#load mesh data
def LoadMeshData(model):
    
    matList = []
    texList = []
    lsffile = []
    All_Mesh_Materials = []              
    Bone_Translations = []
    ExcludeList = [] 
    MatCount = 0  
    
    SKIP_PATH = rapi.getDirForFilePath(noesis.getMainPath())
    
    if rapi.checkFileExists(SKIP_PATH + 'mesh.txt') and SKIP_MESH:
        fm = open(SKIP_PATH + 'mesh.txt',"r")
        lines = fm.read().splitlines()
        for line in lines:
            if line:
                ExcludeList.append(line)      
        
    if MULTIFILE == 1 or MULTIFILE == 2:
        #Intended merged files where skeleton dosent necessarily have all bones in mesh bindings
        if model.Bones:
            for Matrix in model.Bones.InverseWorldTransform:                       
                    boneMat = NoeMat44( [(Matrix[0],Matrix[1],Matrix[2],Matrix[3]), 
                                         (Matrix[4],Matrix[5],Matrix[6],Matrix[7]), 
                                         (Matrix[8],Matrix[9],Matrix[10],Matrix[11]), 
                                         (Matrix[12],Matrix[13],Matrix[14],Matrix[15])] )
         
                    boneMat = boneMat.toMat43().inverse()
                    tran = [boneMat[3][0],boneMat[3][1],boneMat[3][2]]
                    Bone_Translations.append(tran)

    for i in range(len(model.Meshes)):
                   
        Normals = []
        UV1 = []
        UV2 = []
        Tangents = []
        BoneIndices = []
        BoneWeights = []
        Indices = []
        matListTemp = []
        texListTemp = []
        MeshExists = False
        
        Model_Name = model.Meshes[i].info.OrginalModelName
        Vertex_Count = model.Meshes[i].info.Vertex_Count
        Face_Count = model.Meshes[i].info.Face_Count
        Mesh_Name = model.Meshes[i].info.Mesh_Name
        Polygroups = model.Meshes[i].info.Polygroups
        
        if ExcludeList:
            SkipMesh = 0
            if Mesh_Name in ExcludeList and SKIP_MESH == 1:
                continue
            
            if SKIP_MESH == 2:
                for line in ExcludeList:
                    if Mesh_Name.startswith(line):
                        SkipMesh = 1
                        break
           
            if SKIP_MESH == 3:
                for line in ExcludeList:
                    if Mesh_Name.endswith(line):
                        SkipMesh = 1
                        break
                        
            if SkipMesh == 1:
                continue

        if not Model_Name:
            Model_Name = rapi.getLocalFileName(rapi.getInputName())

        Positions =  model.Meshes[i].mesh.Positions
        if model.Meshes[i].mesh.BoneWeights:
            BoneWeights = model.Meshes[i].mesh.BoneWeights
            
            #allows for any number of weights - if less than 4 weights will simply add 0 to complete extra weighs
            if len(BoneWeights[0]) != 4:
                num = 4 - len(BoneWeights[0])
                for weight in BoneWeights:
                    for bw in range(num):
                        weight.append(0)
                        
        if model.Meshes[i].mesh.Normals:
            Normals = model.Meshes[i].mesh.Normals
            if len(Normals[0]) != 3 and not Model_Name.endswith('.model'):
                Normals = []
                
        if model.Meshes[i].mesh.TextureCoordinates:
            UV1 = model.Meshes[i].mesh.TextureCoordinates
            if len(UV1[0]) != 2:
                UV1 = []
                
        if model.Meshes[i].mesh.TextureCoordinates2:
            UV2 = model.Meshes[i].mesh.TextureCoordinates2
            if len(UV2[0]) != 2:
                UV2 = []

        if model.Meshes[i].mesh.Tangents:
            Tangents = model.Meshes[i].mesh.Tangents
             
        #apply correction to from local bone index to global skeleton - assume we have Bone Indices  + weights + bone bindings
        if model.Meshes[i].mesh.BoneIndices and model.Bones:
            NumIndexPerBone = len(model.Meshes[i].mesh.BoneIndices[0])
            BoneIndices = model.Meshes[i].mesh.BoneIndices
                            
            for Count,index in enumerate(BoneIndices):                 
                NewIndex = []
                for j in range(4):
                    if j >= NumIndexPerBone:
                        NewIndex.append(0)
                        continue
                    name = model.Meshes[i].mesh.BoneBindings[index[j]]

                    if name in (model.Bones.BoneNames):                  
                        NewIndex.append(model.Bones.BoneNames.index(name))
                        
                    else:
                        NewIndex.append(0)
                        BoneWeights[Count][j] = 0
                        
                if MULTIFILE == 1 or MULTIFILE == 2:
                                       
                    #when max is 0 all 4 indexs are 0 which means that all refrence bone didnt exist, now try to find closest bone to current vertcie
                    if max(NewIndex) == 0 and Bone_Translations:                       
                        ref = Positions[Count]                    
                        candidates = Bone_Translations[:]                       
                        candidates.sort(key=lambda x: (x[0] - ref[0]) ** 2 + (x[1] - ref[1]) ** 2 + (x[2] - ref[2]) ** 2)                       
                        boneIndex = Bone_Translations.index(candidates[0])                      
                        BoneWeights[Count][0] = 1
                        BoneWeights[Count][1] = BoneWeights[Count][2] = BoneWeights[Count][3] = 0
                        NewIndex[0] = boneIndex                                   
                    else:
                        #If we have bone index that exists but all weights are 0 give boneid a value of 1
                        if max(BoneWeights[Count]) == 0:                     
                            BoneWeights[Count][NewIndex.index(max(NewIndex))] = 255
                         
                BoneIndices[Count] = NewIndex           
                
        #if we have a bone bindings but no weights assume full weight to first bone index
        if model.Meshes[i].mesh.BoneBindings  and BoneIndices and not BoneWeights and model.Bones:
                for num in range(len(Positions)):
                    weights = [255,0.0,0.0,0.0]                    
                    BoneWeights.append(weights)

                    
        #if we have a bone bindings but no weights and no Bone Indices - assume rigid mesh that should be skinned to this bone
        if model.Meshes[i].mesh.BoneBindings and not BoneWeights and not BoneIndices and model.Bones: 
                #should have only one bone
                for boneName in model.Meshes[i].mesh.BoneBindings:
                    for num in range(len(Positions)):
                        index = [0,0,0,0]                       
                        weights = [255,0.0,0.0,0.0]
                        if boneName in model.Bones.BoneNames:
                            index[0] = model.Bones.BoneNames.index(boneName)
                        else:
                            index[0] = 0
                            print("Bone Binding does not exist in Skeleton")
                        BoneIndices.append(index)
                        BoneWeights.append(weights)
        
        
        if GAME_TAG_BG3:
            
            #check if we already have data for this mesh name
            if All_Mesh_Materials:
                for Mat_data in All_Mesh_Materials:
                    if MeshExists:
                        break

                    for MeshName in Mat_data.MeshName:
                        if Mesh_Name in MeshName :
                            MeshExists = True
                            break

            if not MeshExists:
                lsffile, lsf_data = getLSFfile(Mesh_Name,lsffile)        
                All_Mesh_Materials = ConstructMaterials(lsf_data,All_Mesh_Materials)
            
                if All_Mesh_Materials:
                    #Try and locate missing textures for materialID in All_Mesh_Materials
                    Found = 0
                    for Mat in All_Mesh_Materials:
                        #for the mesh name we want, if we have a materialID and no texture id/path attemet to find it
                        if Mat.MatrialID and  SearchTexID(Mat.TextureID) and FindSubStringInList(Mesh_Name,Mat.MeshName):
                            while lsf_data:
                                New = []
                                # we can try first looking in All_Mesh_Materials, and then look for new lsf - now just process new lsf
                                lsffile, lsf_data = getLSFfile(Mesh_Name,lsffile)
                                New_Materials = ConstructMaterials(lsf_data,New)
                                if New_Materials:
                                    for DelIndxe,New in enumerate(New_Materials):
                                        if New.MatrialID == Mat.MatrialID and not SearchTexID(New.TextureID):
                                           Mat.TextureID += New.TextureID
                                           Mat.TexturePath += New.TexturePath
                                           Mat.MeshName += New.MeshName
                                           #remove any possible duplicate names
                                           Mat.MeshName = list(dict.fromkeys( Mat.MeshName))
                                           del New_Materials[DelIndxe]
                                           Found = 1
                                           break
                                    All_Mesh_Materials += New_Materials
                                    if Found == 1:
                                       Found = 0
                                       break   
            
            #check if a material exists that alraedy has textures for new mesh and set existing material to new mesh
            for Mesh_Mat in All_Mesh_Materials:
                for Mat in matList:              
                    if FindSubStringInList(Mat.name,Mesh_Mat.MeshName) and FindSubStringInList(Mesh_Name,Mesh_Mat.MeshName):
                        matListTemp.append(Mat)
                       
            if All_Mesh_Materials and not matListTemp:                      
                texListTemp, matListTemp = GetTextures(i,All_Mesh_Materials,Mesh_Name,texList)    
                matList += matListTemp
                texList += texListTemp

                                    
        baseName = rapi.getLocalFileName(rapi.getInputName())       
        extension = baseName[baseName.index('.') + 1:]
        if extension == 'model':
            #for D2R multiple materials per polygroup
            MatCount = 0
            for MeshMat in model.Meshes[i].info.TextureMaps:
                texListTemp,material  = GetTexturesDS2(MeshMat,Mesh_Name,MatCount,texList)
                if texListTemp:
                    texList += texListTemp 
                if material:
                   matListTemp.append(material)
                   MatCount += 1
                
            matList += matListTemp
        
        if model.Meshes[i].info.Materials:
             MatCount = 0
             for MeshMat in model.Meshes[i].info.Materials:
                if GAME_TAG_SH5:
                    texListTemp,material  = GetTexturesSH5(MeshMat.Maps,Mesh_Name,MatCount,texList)
                elif GAME_TAG_GRANNY:
                    texListTemp,material  = GetGrannyTextures(MeshMat.Maps,Mesh_Name,MatCount,texList)
                if texListTemp:
                    texList += texListTemp 
                if material:
                   matListTemp.append(material)
                   MatCount += 1
        
             matList += matListTemp
            
        Indices = model.Meshes[i].mesh.Indices
        if Tangents and len(Tangents[0]) != 3:
            Tangents = []
            
        NewBytes = CreateBytesData(Positions,Normals,UV1,UV2,Tangents,BoneIndices,BoneWeights,Indices)
                     
        ms = NoeBitStream(NewBytes, NOE_LITTLEENDIAN) 
        ms.seek(0,0)
                
        VertBuff = ms.readBytes(len(Positions) * 12) 
        rapi.rpgBindPositionBufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 12, 0) 
          
        if Normals:
            VertBuff = ms.readBytes(len(Normals) * 12)
            if DEBUG_NORMALS:
                rapi.rpgBindColorBufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 12, 0, 3) #last 3 is for rgb, if i had 4 floats could be set to 4=rgba
            else:
                rapi.rpgBindNormalBufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 12, 0)
        
        if UV1:
            VertBuff = ms.readBytes(len(UV1) * 8)
            rapi.rpgBindUV1BufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 8, 0)
        
        if UV2:
            VertBuff = ms.readBytes(len(UV2) * 8)
            rapi.rpgBindUV2BufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 8, 0)
            
        #3 floats per tangent and a 1 for 4th value
        if Tangents:
            VertBuff = ms.readBytes(len(Tangents) * 16)
            rapi.rpgBindTangentBufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 16, 0)

        if BoneIndices:
            VertBuff = ms.readBytes(len(BoneIndices) * 16)
            rapi.rpgBindBoneIndexBufferOfs(VertBuff, noesis.RPGEODATA_UINT, 16, 0, 4)
            
        if BoneWeights:
            VertBuff = ms.readBytes(len(BoneWeights) * 16)
            rapi.rpgBindBoneWeightBufferOfs(VertBuff, noesis.RPGEODATA_FLOAT, 16, 0, 4)
            
        if len(matListTemp) == 1:
            if matListTemp:
                rapi.rpgSetMaterial(matListTemp[0].name)
                    
                        
        for j in range(len(Polygroups)):
            
            #for D2R set Meterials per polygroup 
            if matListTemp and len(matListTemp) > 1 and len(matListTemp) == len(Polygroups):
                rapi.rpgSetMaterial(matListTemp[j].name)
                
            FaceBuff = ms.readBytes(Polygroups[j] * 3 * 4)
            rapi.rpgSetName(Mesh_Name + "_" + str(j))
            
            if LOAD_POINT_CLOUD:
                rapi.rpgCommitTriangles(None, noesis.RPGEODATA_UINT, Vertex_Count, noesis.RPGEO_POINTS, 1)
            else:
                rapi.rpgCommitTriangles(FaceBuff, noesis.RPGEODATA_UINT, Polygroups[j] * 3, noesis.RPGEO_TRIANGLE, 1)
            
        if MESH_OPTIMIZE:
            rapi.rpgOptimize()
        if S_NORMALS:
            rapi.rpgSmoothNormals()
        rapi.rpgClearBufferBinds()

    return texList, matList


def animation(model,bones, AnimMode, Main_Model,frameRate):
    
    frames = []
    anims = []  
    ParentIndex = []
    Linear3x3 = []
    InverseLinear3x3 = []


    for i in range(len(bones)):
        ParentIndex.append(bones[i].parentIndex)

    if AnimMode == 1:
       Models = GR2Reader(model)
       #find track name the belongs to model name
       for i,mo in enumerate(Models):
            if mo.Name == Main_Model.Name:
                model = Models[i]
                break
            else:
                model = Models[0]

       BoneNameArray = []
       BoneNameArray = Main_Model.Bones.BoneNames
            
    if AnimMode == 2:
       BoneNameArray = model.Bones.BoneNames
       
    if model and model.OrinationTransform.Linear3x3:
        Linear = model.OrinationTransform.Linear3x3
        InverseLinear = model.OrinationTransform.InverseLinear3x3
        Affine3 = model.OrinationTransform.Affine3

        for i in range(3):
            for j in range(3):
                Linear3x3.append(Linear[i][j])
                InverseLinear3x3.append(InverseLinear[i][j])

    if model.Trackgroups:

        Duration = model.AnimationInfo.Duration 
        TimeStep = model.AnimationInfo.TimeStep 
        Oversampling = model.AnimationInfo.Oversampling
        
        #some models have trackgroups with no transform tracks
        if not hasattr(model.Trackgroups, "TransformTracks"):
            return [],1
            
        if  not MERGE_SCENE:
            InitialPlacement = model.Trackgroups.InitialPlacement
            InitialPlacement = ComposeLocalMatrix(InitialPlacement)
            if Linear3x3:
                InitialPlacement = TransformTrackGroupSystem(InitialPlacement,'InitialPlacement',Linear3x3,InverseLinear3x3,Affine3)
            
        for Track in model.Trackgroups.TransformTracks:
             
             #if animation contains a bone that is not part of skeleton i ignore that bone
             if Track.Name not in BoneNameArray:
                    continue
                    
             #get index of wanted bone
             Bindex = BoneNameArray.index(Track.Name)  
            
             #get locale transformes - will be used for bones that are static during animation
             if AnimMode == 1:
                 Quaterion = Main_Model.Bones.Transform[Bindex].Quaterion
                 Translation = Main_Model.Bones.Transform[Bindex].Translation
                 ScaleShear = Main_Model.Bones.Transform[Bindex].ScaleShear
                
             if AnimMode == 2:
                 Quaterion = model.Bones.Transform[Bindex].Quaterion
                 Translation = model.Bones.Transform[Bindex].Translation
                 ScaleShear = model.Bones.Transform[Bindex].ScaleShear
             
             Dummy = Transform_Tracks()
             Dummy.Name = Track.Name  

             if type(Track.OrientationCurve) == daidentity:
                Dummy.OrientationCurve.Controls = [list(Quaterion)]
                Dummy.OrientationCurve.Knots = [0.0]
                Dummy.OrientationCurve.Knots = [0.0]
             else:
                Dummy.OrientationCurve.Controls, Dummy.OrientationCurve.Knots = GetRotation(Track.OrientationCurve)
                if Linear3x3:
                    Dummy.OrientationCurve.Controls = TransformTrackGroupSystem(Dummy.OrientationCurve.Controls,'Rotation',Linear3x3,InverseLinear3x3,Affine3)
                if not Dummy.OrientationCurve.Controls:
                    Dummy.OrientationCurve.Controls = [list(Quaterion)]
                    Dummy.OrientationCurve.Knots = [0.0]

             if type(Track.PositionCurve) == daidentity:
                Dummy.PositionCurve.Controls = [list(Translation)]
                Dummy.PositionCurve.Knots = [0.0]
             else:
                Dummy.PositionCurve.Controls , Dummy.PositionCurve.Knots = GetTranslation(Track.PositionCurve)
                if Linear3x3:
                    Dummy.PositionCurve.Controls = TransformTrackGroupSystem(Dummy.PositionCurve.Controls,'Position',Linear3x3,InverseLinear3x3,Affine3)
                if not Dummy.PositionCurve.Controls:
                    Dummy.PositionCurve.Controls = [list(Translation)]
                    Dummy.PositionCurve.Knots = [0.0]
                        
             if type(Track.ScaleShearCurve) == daidentity:
                Dummy.ScaleShearCurve.Controls = [list(ScaleShear)]
                Dummy.ScaleShearCurve.Knots = [0.0]
             else:
                Dummy.ScaleShearCurve.Controls , Dummy.ScaleShearCurve.Knots  = Track.ScaleShearCurve.GetMartix()
                if Linear3x3:
                    Dummy.ScaleShearCurve.Controls = TransformTrackGroupSystem(Dummy.ScaleShearCurve.Controls,'ScaleShear',Linear3x3,InverseLinear3x3,Affine3)
                if not Dummy.ScaleShearCurve.Controls:
                    Dummy.ScaleShearCurve.Controls = [list(ScaleShear)]
                    Dummy.ScaleShearCurve.Knots = [0.0]
             
             Frame = CreateKeyFrame(Dummy)
             Frame = FrameInterpolate(Frame)
             Frame = ApplyShear(Frame)
             frames.append(Frame)
        
                
        frames = EqualizeFrames(frames,Duration,TimeStep)

        if frames:
            animFrameMats = []
            Numframes = len(frames[0])
            NumBones1 = len(BoneNameArray)
            NumBones2 = len(frames)
            
            #create list of bones used in animation
            Names = []
            for k in range(NumBones2):
                Names.append(frames[k][0].Name)
               
            #create a list length of all bones in skeleton and put each "bone frames" in the correct order as in skeleton 
            BoneFramelist = [-1] * NumBones1
            for k,name in enumerate(Names):
                Bindex = BoneNameArray.index(name)
                BoneFramelist[Bindex] = frames[k]
            
            for f in range(Numframes): 
                value = 0
                for i,Bone in enumerate(BoneFramelist):
                    #Bone is in bone list but not used in animation - i assign of locale skeleton bone
                    if Bone == -1:
                        if AnimMode == 1:
                            boneMat = ComposeLocalMatrix(Main_Model.Bones.Transform[i]) 
                            
                        if AnimMode == 2:
                            boneMat = ComposeLocalMatrix(model.Bones.Transform[i])
                    else:   
                        boneMat = Bone[f].Matrix
                        
                    #assume that bone in i ==0 is root bone, add check in the future
                    if i == 0 and not MERGE_SCENE:
                        boneMat = boneMat * InitialPlacement

                    if Bone == -1 and ParentIndex[i] == -1 and MERGE_SCENE:
                        boneMat = bones[i].getMatrix()
                        
                    #if bone is part of animation and is a root bone
                    if Bone != -1 and ParentIndex[i] == -1 and MERGE_SCENE:
                        InitialPlacement = model.Trackgroups.InitialPlacement[value]
                        value += 1
                        InitialPlacement = ComposeLocalMatrix(InitialPlacement)
                        if Linear3x3:
                            InitialPlacement = TransformTrackGroupSystem(InitialPlacement,'InitialPlacement',Linear3x3,InverseLinear3x3,Affine3)
                        boneMat = boneMat * InitialPlacement
                        
                    animFrameMats.append(boneMat)
            if Numframes/Duration >=  frameRate:
                frameRate = Numframes/Duration
            anim = NoeAnim("Anim", bones, Numframes, animFrameMats,frameRate )
            #used to set frame rate speed in noesis screen
            rapi.setPreviewOption("setAnimSpeed", str(int(frameRate)))
            anims.append(anim) 

        return anims,frameRate
    
    
def noepyLoadModel(data, mdlList):
        
        All_Models = []
        Models = []
        anim_data = []
        anims = []
        bones = []
        frameRate = 1
        
        baseName = rapi.getLocalFileName(rapi.getInputName())
        M_extension = baseName[baseName.index('.') + 1:]
        extension = M_extension
        
        if ANIMATION_MODE == 1:
            if M_extension == 'model':
                extension = 'animations'
            anim_data = rapi.loadPairedFileOptional("Animation_Data", extension)
            
        if SKELETON_LOAD == 1:
           if M_extension == 'model':
                extension = 'skeleton'
           skel_data = rapi.loadPairedFileOptional("Skeleton_Data", extension)

        if MULTIFILE:
        
        #load all .gr2 files in directory
          Combined = Model()
          MeshDir = noesis.userPrompt(noesis.NOEUSERVAL_FOLDERPATH, "Open Folder", "Select the folder that containes wanted .GR2 files")        

          for fileName in os.listdir(MeshDir):
               lowerName = fileName.lower()
               if lowerName.endswith(".gr2") or lowerName.endswith(".fgx") or lowerName.endswith(".model") or lowerName.endswith(".skeleton"):
                    with open(os.path.join(MeshDir, fileName), 'r') as f:                                    
                        MeshFilePath = os.path.join(MeshDir, fileName)
                        gr2Temp = open(MeshFilePath,"rb")
                        TempData = gr2Temp.read()
                        TModels = GR2Reader(TempData)
                        
                        #insert model name to each mesh
                        for model in TModels:
                            if model.Meshes:
                                for mesh in model.Meshes: 
                                    mesh.info.OrginalModelName = fileName

                        All_Models += TModels
                       
          #copy all meshes and skeleton data to a single model
          for model in All_Models:
            setattr(model.Bones, 'Meshes', [])           
            model.Bones.Meshes = []        
            if model.Meshes:
                for mesh in model.Meshes:      
                    model.Bones.Meshes.append(mesh.info.Mesh_Name)
                  
            if MULTIFILE == 1 or MULTIFILE == 2:
                Combined.Meshes += model.Meshes
                
            if MULTIFILE == 1 or MULTIFILE == 3:
                Combined.Bones.append(model.Bones)
                
            Combined.InitialPlacement = model.InitialPlacement
            
          Models.append(Combined)

          if MULTIFILE == 1 or MULTIFILE == 3:
              bones = MergeSkeletons2(Combined)
                  
        else:
            Models = GR2Reader(data)  
        
        if SKELETON_LOAD == 1 and skel_data:
            Skeleton = GR2Reader(skel_data)
            Models[0].Bones = Skeleton[0].Bones
        
        #condition when format is not supported and Models is equal to 0
        if Models == 0:
            return 0
            
        for model in Models:
            ctx = rapi.rpgCreateContext()

            if model.Bones and (MULTIFILE == 0 or MULTIFILE == 2):
                if MERGE_SCENE ==1:
                    bones = MergeSkeletons(model)
                    model.bones = UpdatePostMerge(model)
                else:
                    bones = SkeletonLocale(model)

            if model.Meshes:
                texList, matList = LoadMeshData(model)
                mdl = rapi.rpgConstructModel()
                if matList:
                    mdl.setModelMaterials(NoeModelMaterials(texList, matList))
            else:
                mdl = NoeModel()

            if anim_data and bones:
                    anims,frameRate = animation(anim_data,bones, ANIMATION_MODE, model,frameRate)
                    
            elif ANIMATION_MODE == 2 and bones and len(bones) > 1 and model.Trackgroups:
                    anims,frameRate = animation(model,bones, ANIMATION_MODE, None,frameRate)

            if bones:
                mdl.setBones(bones)
                
            if anims:
                mdl.setAnims(anims)
            
            #make sure i load only models that contain mesh and/or skeleton data
            if not bones and not model.Meshes:
                continue
                
            mdlList.append(mdl)

        return 1 