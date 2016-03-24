﻿


#include "genLUA.h"

std::string GenLUA::genRealContent(const std::list<AnyData> & stores)
{
    std::string text;
    for (auto &info : stores)
    {
        if (info._type == GT_DataConstValue)
        {
            text += LFCR;
            text += genDataConst(info._const);
        }
        else if (info._type == GT_DataEnum)
        {
            text += LFCR;
            text += genDataEnum(info._enum);
        }
        else if (info._type == GT_DataArray)
        {
            text += LFCR;
            text += genDataArray(info._array);
        }
        else if (info._type == GT_DataMap)
        {
            text += LFCR;
            text += genDataMap(info._map);
        }
        else if (info._type == GT_DataStruct || info._type == GT_DataProto)
        {
            text += LFCR;
            text += genDataProto(info._proto, info._type == GT_DataProto);
        }
    }

    return std::move(text);
}



std::string GenLUA::genDataConst(const DataConstValue & dc)
{
    std::string text;
    text += "Proto4z." + dc._name + " = " + dc._value;
    if (!dc._desc.empty())
    {
        text += "--" + dc._desc;
    }
    return text;
}

std::string GenLUA::genDataEnum(const DataEnum & de)
{
    std::string text;
    for (const auto & m : de._members)
    {
        text += "Proto4z." + m._name + " = " + m._value;
        if (!m._desc.empty())
        {
            text += "--" + m._desc;
        }
        text += LFCR;
    }
    return text;
}
std::string GenLUA::genDataArray(const DataArray & da)
{
    std::string text;
    text += "Proto4z." + da._arrayName + " = {} ";
    if (!da._desc.empty())
    {
        text += "--" + da._desc;
    }
    text += LFCR;

    text += "Proto4z." + da._arrayName + ".__getName = \"" + da._arrayName + "\"" + LFCR;
    text += "Proto4z." + da._arrayName + ".__getDesc = \"array\"" + LFCR;
    text += "Proto4z." + da._arrayName + ".__getTypeV = \"" + da._type + "\"" + LFCR;
    return text;
}
std::string GenLUA::genDataMap(const DataMap & dm)
{
    std::string text;
    text += "Proto4z." + dm._mapName + " = {} ";
    if (!dm._desc.empty())
    {
        text += "--" + dm._desc;
    }
    text += LFCR;

    text += "Proto4z." + dm._mapName + ".__getName = \"" + dm._mapName + "\"" + LFCR;
    text += "Proto4z." + dm._mapName + ".__getDesc = \"map\"" + LFCR;
    text += "Proto4z." + dm._mapName + ".__getTypeK = \"" + dm._typeKey + "\"" + LFCR;
    text += "Proto4z." + dm._mapName + ".__getTypeV = \"" + dm._typeValue + "\"" + LFCR;
    return text;
}
std::string GenLUA::genDataProto(const DataProto & dp, bool isProto)
{
    std::string text;
    if (isProto)
    {
        text += "Proto4z.register(" + dp._const._value + ",\"" + dp._struct._name + "\")" + LFCR;
    }

    text += "Proto4z." + dp._struct._name + " = {} ";
    if (!dp._struct._desc.empty())
    {
        text += "--" + dp._struct._desc;
    }
    text += LFCR;

    if (isProto)
    {
        text += "Proto4z." + dp._struct._name + ".__getID = " + dp._const._value + "" + LFCR;
    }
    text += "Proto4z." + dp._struct._name + ".__getName = \"" + dp._struct._name + "\"" + LFCR;

    for (size_t i = 0; i < dp._struct._members.size(); ++i)
    {
        text += "Proto4z." + dp._struct._name + "[" + toString(i + 1)
            + "] = {name=\"" + dp._struct._members[i]._name + "\", type=\"" + dp._struct._members[i]._type + "\"";
        text += " } ";
        if (!dp._struct._members[i]._desc.empty())
        {
            text += "--" + dp._struct._members[i]._desc;
        }
        text += LFCR;
    }
    return text;
}


