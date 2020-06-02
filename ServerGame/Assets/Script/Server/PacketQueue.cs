using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Runtime.InteropServices;
[System.Serializable]
public class PacketQueue
{
    //패킷 저장 정보
    struct PacketInfo
    {
        public int offset;
        public int size;
    };

    //MemoryStream : 스트림 데이터를 파일이나 소켓대신 메모리에 직접 출력한다
    private MemoryStream m_streamBuffer;
    private List<PacketInfo> m_offsetList;
    private int m_offset = 0;

    public PacketQueue()
    {
        m_streamBuffer = new MemoryStream();
        m_offsetList = new List<PacketInfo>();
    }

    public int Enqueue(byte[] data, int size)
    {
        PacketInfo info = new PacketInfo();

        info.offset = m_offset;
        info.size = size;

        //패킷 저장 정보를 보존
        m_offsetList.Add(info);

        //패킷 데이터를 보존
        m_streamBuffer.Position = m_offset;     //스트림 내의 현재 위치를 가져오거나 설정한다
        m_streamBuffer.Write(data, 0, size);    //버퍼에서 읽은 데이터를 사용하여 현재 스트림에 바이트 블록을 쓴다
        m_streamBuffer.Flush();                 //매서드를 재정의하여 아무런 작업도 수행되지 않도록 한다
        m_offset += size;

        return size;
    }

    public int Dequeue(ref byte[] buffer, int size)
    {
        if (m_offsetList.Count <= 0)
        {
            return -1;
        }

        PacketInfo info = m_offsetList[0];

        //버퍼에서 해당하는 패킷 데이터를 가져온다
        int dataSize = Math.Min(size, info.size);
        m_streamBuffer.Position = info.offset;
        int recvSize = m_streamBuffer.Read(buffer, 0, dataSize);

        //큐 데이터를 가져왔으므로 선두 요소를 삭제
        if (recvSize > 0)
        {
            m_offsetList.RemoveAt(0);
        }

        //모든 큐 데이터를 가져왔을 때는 스트림을 클리어해서 메모리를 절약
        if (m_offsetList.Count == 0)
        {
            Clear();
            m_offset = 0;
        }
        return recvSize;
    }

    public void Clear()
    {
        byte[] buffer = m_streamBuffer.GetBuffer();
        Array.Clear(buffer, 0, buffer.Length);

        m_streamBuffer.Position = 0;
        m_streamBuffer.SetLength(0);
    }
}
