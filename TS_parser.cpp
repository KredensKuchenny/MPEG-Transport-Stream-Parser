#include "tsCommon.h"
#include "tsTransportStream.h"
#include <iostream>
#include <cstdio>

//=============================================================================================================================================================================

int main(int argc, char *argv[], char *envp[])
{

  const char *fileNamePID136 = "PID136.mp2";

  if (std::remove(fileNamePID136) == 0)
  {
    printf("The file '%s' has been deleted.\n", fileNamePID136);
  }
  else
  {
    perror("Error occurred while deleting the file.\n");
  }

  const char *fileNamePID174 = "PID174.264";

  if (std::remove(fileNamePID174) == 0)
  {
    printf("The file '%s' has been deleted.\n", fileNamePID174);
  }
  else
  {
    perror("Error occurred while deleting the file.\n");
  }

  // TODO - open file | done
  FILE *fp;
  fp = fopen("example_new.ts", "rb");

  FILE *
      filePID136 = fopen("PID136.mp2", "ab");

  FILE *
      filePID174 = fopen("PID174.264", "ab");

  // TODO - check if file if opened | done
  if (fp != NULL)
  {
    printf("File exists\n\n");
  }
  else
  {
    printf("File does not exists\n\n");
    return 0;
  }

  if (filePID136 == nullptr || filePID174 == nullptr)
  {
    std::cout << "The file cannot be opened.\n\n";
    return 1;
  }

  xTS_PacketHeader TS_PacketHeader;
  xTS_AdaptationField TS_PacketAdaptationField;
  xPES_PacketHeader PES_PacketHeader;
  xPES_Assembler PES_Assembler_PID136;
  xPES_Assembler PES_Assembler_PID174;
  xTS packet;

  int32_t TS_PacketId = 0;
  while (!feof(fp))
  {
    int offset = 0;

    // TODO - read from file | done
    // const int iRozmiarBufora = 188; //po 188 bajtow
    uint8_t bufor[packet.TS_PacketLength]; // rzutowanie char-a na uint8_t
    // int iOdczytanoBajtow = fread(bufor, sizeof(char), packet.TS_PacketLength, fp);
    // printf( "Odczytano %d bajtow.\n", iOdczytanoBajtow );

    size_t NumRead = fread(bufor, 1, packet.TS_PacketLength, fp);
    if (NumRead != packet.TS_PacketLength)
    {
      break;
    }

    TS_PacketHeader.Reset();
    offset += TS_PacketHeader.Parse(bufor);

    if (TS_PacketHeader.getSyncByte() == 'G' && (TS_PacketHeader.getPID() == 136 || TS_PacketHeader.getPID() == 174))
    {
      if (TS_PacketHeader.hasAdaptationField())
      {
        TS_PacketAdaptationField.Reset();
        offset += TS_PacketAdaptationField.Parse(bufor + offset, TS_PacketHeader.getAdaptationFieldControl());
      }

      printf("%010d ", TS_PacketId);
      TS_PacketHeader.Print();

      if (TS_PacketHeader.hasAdaptationField())
      {
        TS_PacketAdaptationField.Print();
      }

      if (TS_PacketHeader.getPID() == 136)
      {
        xPES_Assembler::eResult Result = PES_Assembler_PID136.AbsorbPacket(bufor + offset, &TS_PacketHeader, &TS_PacketAdaptationField);
        switch (Result)
        {
        case xPES_Assembler::eResult::StreamPackedLost:
          printf("PcktLost\n");
          break;
        case xPES_Assembler::eResult::AssemblingStarted:
          printf("Started\n");
          PES_Assembler_PID136.PrintPESH();
          fwrite(PES_Assembler_PID136.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID136);
          break;
        case xPES_Assembler::eResult::AssemblingContinue:
          printf("Continue\n");
          fwrite(PES_Assembler_PID136.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID136);
          break;
        case xPES_Assembler::eResult::AssemblingFinished:
          printf("Finished\n");
          printf("PES: Len=%d", PES_Assembler_PID136.getNumPacketBytes());
          fwrite(PES_Assembler_PID136.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID136);
          break;
        default:
          break;
        }
      }
      else if (TS_PacketHeader.getPID() == 174)
      {
        xPES_Assembler::eResult Result = PES_Assembler_PID174.AbsorbPacket(bufor + offset, &TS_PacketHeader, &TS_PacketAdaptationField);
        switch (Result)
        {
        case xPES_Assembler::eResult::StreamPackedLost:
          printf("PcktLost\n");
          break;
        case xPES_Assembler::eResult::AssemblingStarted:
          printf("Started\n");
          PES_Assembler_PID174.PrintPESH();
          fwrite(PES_Assembler_PID174.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID174);
          break;
        case xPES_Assembler::eResult::AssemblingContinue:
          printf("Continue\n");
          fwrite(PES_Assembler_PID174.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID174);
          break;
        case xPES_Assembler::eResult::AssemblingFinished:
          printf("Finished\n");
          printf("PES: Len=%d", PES_Assembler_PID174.getNumPacketBytes());
          fwrite(PES_Assembler_PID174.getPacket(), sizeof(uint8_t), (packet.TS_PacketLength - offset), filePID174);
          break;
        default:
          break;
        }
      }

      printf("\n");
      printf("\n");
    }
    TS_PacketId++;
  }

  // TODO - close file | done
  fclose(fp);
  fclose(filePID136);
  fclose(filePID174);

  return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
