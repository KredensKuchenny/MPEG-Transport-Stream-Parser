#include "tsTransportStream.h"
#include <iostream>
#include <iomanip>

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_PacketHeader::Reset()
{
  this->m_SB = 0;
  this->m_E = 0;
  this->m_S = 0;
  this->m_T = 0;
  this->m_PID = 0;
  this->m_TSC = 0;
  this->m_AFC = 0;
  this->m_CC = 0;
}

/**
  @brief Parse all TS packet header fields | done
  @param Input is pointer to buffer containing TS packet | done
  @return Number of parsed bytes (4 on success, -1 on failure) | done
 */
int32_t xTS_PacketHeader::Parse(const uint8_t *Input)
{
  if (Input != NULL)
  {
    const uint32_t *HeaderPtr = (const uint32_t *)Input;
    uint32_t HeaderData = xSwapBytes32(*HeaderPtr);
    this->m_SB = (uint32_t)((HeaderData & 0b11111111000000000000000000000000) >> 24);
    this->m_E = (uint32_t)((HeaderData & 0b00000000100000000000000000000000) >> 23);
    this->m_S = (uint32_t)((HeaderData & 0b00000000010000000000000000000000) >> 22);
    this->m_T = (uint32_t)((HeaderData & 0b00000000001000000000000000000000) >> 21);
    this->m_PID = (uint32_t)((HeaderData & 0b00000000000111111111111100000000) >> 8);
    this->m_TSC = (uint32_t)((HeaderData & 0b00000000000000000000000011000000) >> 6);
    this->m_AFC = (uint32_t)((HeaderData & 0b00000000000000000000000000110000) >> 4);
    this->m_CC = (uint32_t)((HeaderData & 0b00000000000000000000000000001111));

    return 4;
  }
  else
  {
    return NOT_VALID;
  }
}

/// @brief Print all TS packet header fields
void xTS_PacketHeader::Print() const
{
  // Print the TS packet header fields | done
  std::cout << "\nTS:" << std::endl;
  // std::cout << "\nSync byte: " << std::hex << std::setw(2) << std::setfill('0') << (int)m_SB << std::endl;
  std::cout << "  Sync byte: " << (int)m_SB << std::endl;
  std::cout << "  Transport error indicator: " << (int)m_E << std::endl;
  std::cout << "  Payload unit start indicator: " << (int)m_S << std::endl;
  std::cout << "  Transport priority: " << (int)m_T << std::endl;
  // std::cout << "PID: " << std::hex << std::setw(4) << std::setfill('0') << (int)m_PID << std::endl;
  std::cout << "  PID: " << (int)m_PID << std::endl;
  std::cout << "  Transport scrambling control: " << (int)m_TSC << std::endl;
  std::cout << "  Adaptation field control: " << (int)m_AFC << std::endl;
  std::cout << "  Continuity counter: " << (int)m_CC << std::endl;
}

//=============================================================================================================================================================================

// @brief Reset - reset all TS packet header fields
void xTS_AdaptationField::Reset()
{
  // setup | DONE
  this->m_AdaptationFieldControl = 0;

  // mandatory fields | DONE
  this->m_AdaptationFieldLength = 0;
  this->m_DiscontinuityIndicator = 0;
  this->m_RandomAccessIndicator = 0;
  this->m_ElementaryStreamPriorityIndicator = 0;
  this->m_PCRFlag = 0;
  this->m_OPCRFlag = 0;
  this->m_SplicingPointFlag = 0;
  this->m_TransportPrivateDataFlag = 0;
  this->m_AdaptationFieldExtensionFlag = 0;

  // optional fields - PCR | DONE
  this->m_ProgramClockReferenceBase = 0;
  this->m_ProgramClockReferenceExtension = 0;
  this->m_ProgramClockReference = 0;
  this->m_ProgramClockReferenceTime = 0;

  // optional fields - OPCR | DONE
  this->m_OriginalProgramClockReferenceBase = 0;
  this->m_OriginalProgramClockReferenceExtension = 0;
  this->m_OriginalProgramClockReference = 0;
  this->m_OriginalProgramClockReferenceTime = 0;

  // stuffing bytes | DONE
  this->m_NumStuffingBytes = 0;
}

/**
  @brief Parse adaptation field
  @param PacketBuffer is pointer to buffer containing TS packet
  @param AdaptationFieldControl is value of Adaptation Field Control field of corresponding TS packet header
  @return Number of parsed bytes (length of AF or -1 on failure)
*/

int32_t xTS_AdaptationField::Parse(const uint8_t *PacketBuffer, uint8_t AdaptationFieldControl)
{
  if (PacketBuffer)
  {
    // setup | DONE
    this->m_AdaptationFieldControl = AdaptationFieldControl;

    // mandatory fields | DONE
    this->m_AdaptationFieldLength = PacketBuffer[0];
    this->m_DiscontinuityIndicator = (PacketBuffer[1] & 0b10000000) >> 7;
    this->m_RandomAccessIndicator = (PacketBuffer[1] & 0b01000000) >> 6;
    this->m_ElementaryStreamPriorityIndicator = (PacketBuffer[1] & 0b00100000) >> 5;
    this->m_PCRFlag = (PacketBuffer[1] & 0b00010000) >> 4;
    this->m_OPCRFlag = (PacketBuffer[1] & 0b00001000) >> 3;
    this->m_SplicingPointFlag = (PacketBuffer[1] & 0b00000100) >> 2;
    this->m_TransportPrivateDataFlag = (PacketBuffer[1] & 0b00000010) >> 1;
    this->m_AdaptationFieldExtensionFlag = (PacketBuffer[1] & 0b00000001);

    xTS packet;

    // optional fields - PCR | DONE
    if (this->m_PCRFlag)
    {
      this->m_ProgramClockReferenceBase = (uint64_t)(PacketBuffer[2] << 25) |
                                          (uint64_t)(PacketBuffer[3] << 17) | (uint64_t)(PacketBuffer[4] << 9) |
                                          (uint64_t)(PacketBuffer[5] << 1) | (uint64_t)(PacketBuffer[6] >> 7);
      this->m_ProgramClockReferenceExtension = (uint16_t)((PacketBuffer[6] & 0b00000001) << 8) | (uint16_t)(PacketBuffer[7]);
      this->m_ProgramClockReference = (m_ProgramClockReferenceBase * 300 + m_ProgramClockReferenceExtension); // PCR(i) = PCR _ base(i) * 300 + PCR _ ext(i) | strona 31 dokumentacja
      this->m_ProgramClockReferenceTime = (float)m_ProgramClockReference / packet.ExtendedClockFrequency_Hz;
    }

    // optional fields - OPCR | DONE
    if (this->m_OPCRFlag)
    {
      this->m_OriginalProgramClockReferenceBase = (uint64_t)(PacketBuffer[8] << 25) |
                                                  (uint64_t)(PacketBuffer[9] << 17) | (uint64_t)(PacketBuffer[10] << 9) |
                                                  (uint64_t)(PacketBuffer[11] << 1) | (uint64_t)(PacketBuffer[12] >> 7);
      this->m_OriginalProgramClockReferenceExtension = (uint16_t)((PacketBuffer[12] & 0b00000001) << 8) | (uint16_t)(PacketBuffer[13]);
      this->m_OriginalProgramClockReference = (m_OriginalProgramClockReferenceBase * 300 + m_OriginalProgramClockReferenceExtension);
      this->m_OriginalProgramClockReferenceTime = (float)m_OriginalProgramClockReference / packet.ExtendedClockFrequency_Hz;
    }

    // calculate stuffing bytes | DONE
    if (this->m_AdaptationFieldLength > 0)
    {
      int numBytes = getNumBytes();
      if (this->m_PCRFlag)
      {
        // subtract PCR bytes
        numBytes -= 6;
      }
      if (this->m_OPCRFlag)
      {
        // subtract OPCR bytes
        numBytes -= 6;
      }
      this->m_NumStuffingBytes = numBytes;
    }
    return m_AdaptationFieldLength + 1;
  }
  else
  {
    return -1;
  }
}

/// @brief Print all TS packet header fields | DONE
void xTS_AdaptationField::Print() const
{
  std::cout << "AF:" << std::endl;
  std::cout << "  Adaptation field length: " << (int)m_AdaptationFieldLength << std::endl;
  std::cout << "  Discontinuity indicator: " << (int)m_DiscontinuityIndicator << std::endl;
  std::cout << "  Random Access indicator: " << (int)m_RandomAccessIndicator << std::endl;
  std::cout << "  Elementary stream priority indicator: " << (int)m_ElementaryStreamPriorityIndicator << std::endl;
  std::cout << "  PCR flag: " << (int)m_PCRFlag << std::endl;
  std::cout << "  OPCR flag: " << (int)m_OPCRFlag << std::endl;
  std::cout << "  Splicing point flag: " << (int)m_SplicingPointFlag << std::endl;
  std::cout << "  Transport private data flag: " << (int)m_TransportPrivateDataFlag << std::endl;
  std::cout << "  Adaptation field extension flag: " << (int)m_AdaptationFieldExtensionFlag << std::endl;
  std::cout << "  Program clock reference base: " << (int)m_ProgramClockReferenceBase << std::endl;
  std::cout << "  Program clock reference extension: " << (int)m_ProgramClockReferenceExtension << std::endl;
  std::cout << "  Program clock reference: " << (int)m_ProgramClockReference << " (Time=" << (float)m_ProgramClockReferenceTime << "s)" << std::endl;
  std::cout << "  Original program clock reference base: " << (int)m_OriginalProgramClockReferenceBase << std::endl;
  std::cout << "  Original program clock reference extension: " << (unsigned int)m_OriginalProgramClockReferenceExtension << " (Time=" << (float)m_OriginalProgramClockReferenceTime << "s)" << std::endl;
  std::cout << "  Stuffing bytes: " << (int)m_NumStuffingBytes << std::endl;
}

//=============================================================================================================================================================================

// @brief Reset - reset all PES packet header fields
void xPES_PacketHeader::Reset()
{
  this->m_PacketStartCodePrefix = 0;
  this->m_StreamId = 0;
  this->m_PacketLength = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t *PacketBuffer)
{
  this->m_PacketStartCodePrefix = PacketBuffer[0] << 16 | PacketBuffer[1] << 8 | PacketBuffer[2];
  this->m_StreamId = PacketBuffer[3];
  this->m_PacketLength = PacketBuffer[4] << 8 | PacketBuffer[5];

  return m_PacketLength;
}

void xPES_PacketHeader::Print() const
{
  std::cout << "PES:" << std::endl;
  std::cout << "  Packet Start Code Prefix: " << (int)m_PacketStartCodePrefix << std::endl;
  std::cout << "  Stream ID: " << (int)m_StreamId << std::endl;
  std::cout << "  Packet Length: " << (int)m_PacketLength << std::endl;
}

//=============================================================================================================================================================================

void xPES_Assembler::xBufferReset()
{
  this->m_Buffer = 0;
  this->m_BufferSize = 0;
  this->m_DataOffset = 0;
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t *TransportStreamPacket, const xTS_PacketHeader *PacketHeader, const xTS_AdaptationField *AdaptationField)
{
  uint8_t temp_BufferSize = 0;

  if (PacketHeader->getPID() == 136)
  {
    if (PacketHeader->getStart())
    {
      xBufferReset();
      this->m_Started = true;
      m_PESH.Reset();
      m_PESH.Parse(TransportStreamPacket);
      this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

      if (m_PESH.getPacketStartCodePrefix() == 0x000001)
      {
        if (m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_program_stream_map &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_padding_stream &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_private_stream_2 &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_ECM &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_EMM &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_program_stream_directory &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_DSMCC_stream &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_ITUT_H222_1_type_E)
        {

          int a = (TransportStreamPacket[7] & 0b11000000) >> 6;

          if (a == 2)
          {
            uint64_t pts = 0;

            uint64_t pts1 = ((TransportStreamPacket[9] & 0b00001110) >> 1);
            uint64_t pts2 = (TransportStreamPacket[10]);
            uint64_t pts3 = ((TransportStreamPacket[11] & 0b11111110) >> 1);
            uint64_t pts4 = (TransportStreamPacket[12]);
            uint64_t pts5 = ((TransportStreamPacket[13] & 0b11111110) >> 1);

            pts = pts1 << 30 | pts2 << 22 | pts3 << 15 | pts4 << 7 | pts5;
            double timePTS = pts / 90000.0;

            std::cout << std::endl;
            std::cout << "  PTS value: " << pts << std::endl;
            std::cout << "  (Time=" << timePTS << "s)" << std::endl;
          }
          else if (a == 3)
          {
            uint64_t pts = 0;
            uint64_t dts = 0;

            uint64_t pts1 = ((TransportStreamPacket[9] & 0b00001110) >> 1);
            uint64_t pts2 = (TransportStreamPacket[10]);
            uint64_t pts3 = ((TransportStreamPacket[11] & 0b11111110) >> 1);
            uint64_t pts4 = (TransportStreamPacket[12]);
            uint64_t pts5 = ((TransportStreamPacket[13] & 0b11111110) >> 1);

            pts = pts1 << 30 | pts2 << 22 | pts3 << 15 | pts4 << 7 | pts5;

            uint64_t dts1 = ((TransportStreamPacket[14] & 0b00001110) >> 1);
            uint64_t dts2 = (TransportStreamPacket[15]);
            uint64_t dts3 = ((TransportStreamPacket[16] & 0b11111110) >> 1);
            uint64_t dts4 = (TransportStreamPacket[17]);
            uint64_t dts5 = ((TransportStreamPacket[18] & 0b11111110) >> 1);

            dts = dts1 << 30 | dts2 << 22 | dts3 << 15 | dts4 << 7 | dts5;

            double timePTS = pts / 90000.0;
            double timeDTS = dts / 90000.0;

            uint64_t ptsDiffDts = pts - dts;
            double timePtsDiffDts = ptsDiffDts / 90000.0;

            std::cout << std::endl;
            std::cout << "  PTS value: " << pts << std::endl;
            std::cout << "  (Time=" << timePTS << "s)" << std::endl;
            std::cout << "  DTS value: " << dts << std::endl;
            std::cout << "  (Time=" << timeDTS << "s)" << std::endl;
            std::cout << "  PTS-DTS value: " << ptsDiffDts << std::endl;
            std::cout << "  (Time=" << timePtsDiffDts << "s)\n"
                      << std::endl;
          }

          if (PacketHeader->hasAdaptationField())
          {
            temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
          }
          else
          {
            temp_BufferSize = 4;
          }
          this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
        }
        else
        {
          if (PacketHeader->hasAdaptationField())
          {
            temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
          }
          else
          {
            temp_BufferSize = 4;
          }

          this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
        }
      }

      this->m_BufferSize = 188 - temp_BufferSize;
      this->m_DataOffset += m_BufferSize;

      return eResult::AssemblingStarted;
    }

    if (this->m_Started)
    {
      if (PacketHeader->hasAdaptationField())
      {
        temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
      }
      else
      {
        temp_BufferSize = 4;
      }

      this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
      this->m_BufferSize = 188 - temp_BufferSize;
      this->m_DataOffset += m_BufferSize;

      if ((PacketHeader->getContinuityCounter() != this->m_LastContinuityCounter + 1) && (PacketHeader->getContinuityCounter() != 0 && this->m_LastContinuityCounter != 15))
      {
        this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

        return eResult::StreamPackedLost;
      }

      this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

      if (this->m_DataOffset >= m_PESH.getPacketLength())
      {
        this->m_Started = false;

        return eResult::AssemblingFinished;
      }
      else
      {
        return eResult::AssemblingContinue;
      }
    }
  }
  else if (PacketHeader->getPID() == 174)
  {
    if (PacketHeader->getStart())
    {
      xBufferReset();
      this->m_Started = true;
      m_PESH.Reset();
      m_PESH.Parse(TransportStreamPacket);
      this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

      if (m_PESH.getPacketStartCodePrefix() == 0x000001)
      {
        if (m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_program_stream_map &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_padding_stream &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_private_stream_2 &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_ECM &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_EMM &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_program_stream_directory &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_DSMCC_stream &&
            m_PESH.getStreamId() != xPES_PacketHeader::eStreamId_ITUT_H222_1_type_E)
        {

          int a = (TransportStreamPacket[7] & 0b11000000) >> 6;

          if (a == 2)
          {
            uint64_t pts = 0;

            uint64_t pts1 = ((TransportStreamPacket[9] & 0b00001110) >> 1);
            uint64_t pts2 = (TransportStreamPacket[10]);
            uint64_t pts3 = ((TransportStreamPacket[11] & 0b11111110) >> 1);
            uint64_t pts4 = (TransportStreamPacket[12]);
            uint64_t pts5 = ((TransportStreamPacket[13] & 0b11111110) >> 1);

            pts = pts1 << 30 | pts2 << 22 | pts3 << 15 | pts4 << 7 | pts5;
            double timePTS = pts / 90000.0;

            std::cout << std::endl;
            std::cout << "  PTS value: " << pts << std::endl;
            std::cout << "  (Time=" << timePTS << "s)" << std::endl;
          }
          else if (a == 3)
          {
            uint64_t pts = 0;
            uint64_t dts = 0;

            uint64_t pts1 = ((TransportStreamPacket[9] & 0b00001110) >> 1);
            uint64_t pts2 = (TransportStreamPacket[10]);
            uint64_t pts3 = ((TransportStreamPacket[11] & 0b11111110) >> 1);
            uint64_t pts4 = (TransportStreamPacket[12]);
            uint64_t pts5 = ((TransportStreamPacket[13] & 0b11111110) >> 1);

            pts = pts1 << 30 | pts2 << 22 | pts3 << 15 | pts4 << 7 | pts5;

            uint64_t dts1 = ((TransportStreamPacket[14] & 0b00001110) >> 1);
            uint64_t dts2 = (TransportStreamPacket[15]);
            uint64_t dts3 = ((TransportStreamPacket[16] & 0b11111110) >> 1);
            uint64_t dts4 = (TransportStreamPacket[17]);
            uint64_t dts5 = ((TransportStreamPacket[18] & 0b11111110) >> 1);

            dts = dts1 << 30 | dts2 << 22 | dts3 << 15 | dts4 << 7 | dts5;

            double timePTS = pts / 90000.0;
            double timeDTS = dts / 90000.0;

            uint64_t ptsDiffDts = pts - dts;
            double timePtsDiffDts = ptsDiffDts / 90000.0;

            std::cout << std::endl;
            std::cout << "  PTS value: " << pts << std::endl;
            std::cout << "  (Time=" << timePTS << "s)" << std::endl;
            std::cout << "  DTS value: " << dts << std::endl;
            std::cout << "  (Time=" << timeDTS << "s)" << std::endl;
            std::cout << "  PTS-DTS value: " << ptsDiffDts << std::endl;
            std::cout << "  (Time=" << timePtsDiffDts << "s)\n"
                      << std::endl;
          }

          if (PacketHeader->hasAdaptationField())
          {
            temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
          }
          else
          {
            temp_BufferSize = 4;
          }

          this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
        }
        else
        {
          if (PacketHeader->hasAdaptationField())
          {
            temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
          }
          else
          {
            temp_BufferSize = 4;
          }

          this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
        }
      }

      this->m_BufferSize = 188 - temp_BufferSize;
      this->m_DataOffset += m_BufferSize;

      return eResult::AssemblingStarted;
    }
    else
    {
      if (PacketHeader->hasAdaptationField())
      {
        temp_BufferSize = 4 + AdaptationField->getAdaptationFieldLength() + 1;
      }
      else
      {
        temp_BufferSize = 4;
      }

      this->m_Buffer = const_cast<uint8_t *>(TransportStreamPacket);
      this->m_BufferSize = 188 - temp_BufferSize;
      this->m_DataOffset += m_BufferSize;

      if ((PacketHeader->getContinuityCounter() != this->m_LastContinuityCounter + 1) && (PacketHeader->getContinuityCounter() != 0 && this->m_LastContinuityCounter != 15))
      {
        this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

        return eResult::StreamPackedLost;
      }

      this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();

      if (PacketHeader->hasAdaptationField())
      {
        this->m_Started = false;

        return eResult::AssemblingFinished;
      }
      else
      {
        return eResult::AssemblingContinue;
      }
    }
  }
  else
  {
    return eResult::UnexpectedPID;
  }
}