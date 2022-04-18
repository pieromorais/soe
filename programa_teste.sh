#! /bin/sh
echo "Shell script"
for i in $(seq 1 10); do
{
      echo "getr";
      sleep 1;
      echo "getn";
      sleep 1;
      echo "1";
      sleep 1;
      echo "rotate";
      sleep 1;
      echo "1";
      sleep 1;
      echo "2";
      sleep 1;
      echo "getr";
      sleep 1;
      echo "alteracoes";
      sleep 1;
      echo "palavras-d";
      sleep 1;
      echo "2";
      sleep 1;
      echo "grava";
      sleep 1;
      echo "replace"
      sleep 1;
      echo "10";
      sleep 1;
      echo "novo ditato";
      sleep 1;
      echo "grava";
      sleep 1;
      echo "fim";
} | telnet localhost 5001
done