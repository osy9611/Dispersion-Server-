using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
public struct PlayerInfo
{
    public string ID;
    public GameObject Charactor;
    public double Rot;
}

[System.Serializable]
public struct MyData
{
    public string type;
    public string ID;
    public string Password;
}

[System.Serializable]
public struct MoveKey
{
    public string type;
    public string ID;
    public string RoomName;
    public double x;
    public double y;
    public double z;
    public double nx;
    public double ny;
    public double nz;
}

[System.Serializable]
public struct InputKey
{
    public string type;
    public string ID;
    public string RoomName;
    public bool Run;
    public bool Attack;
}

public struct NowPosition
{
    public string type;
    public string ID;
    public string RoomName;
    public double nx;
    public double ny;
    public double nz;
    public double Rot;
    public double NowTime;
}

[System.Serializable]
public struct MoveCamera
{
    public string type;
    public string ID;
    public string RoomName;
    public double y;
    public int stop;
}

[System.Serializable]
public struct BossData
{
    public string type;
    public string RoomName;
    public double Damage;
}
public struct Position
{
    public string type;
    public string ID;
    public double x;
    public double y;
    public double z;
    public double Rot;
}

public struct RoomData
{
    public string type;
    public string ID;
    public string RoomName;
    public int Total;
}

[System.Serializable]
public struct EnterData
{
    public string type;
    public string ID;
    public string RoomName;
}

public struct ReadyCheck
{
    public string type;
    public string ID;
    public string RoomName;
    public bool Ready;
}

[System.Serializable]
public struct SpawnPoint
{
    public int Pos;
    public string ID;
}

