#pragma once
/*! \file */
/**
\brief Tryby obs�ugi b��d�w
\ingroup Enumerations
*/
enum ErrorMode : size_t {
    /**
     0 \n
     Wyst�pienie b��du w trakcie realizacji sekwencji powoduje wy�wietlenie komunikatu i zatrzymanie przetwarzania,
     a� do naci�ni�cia klawisza OK \n
     Jest to domy�lny tryb po w��czeniu urz�dzenia.
     */
    DEVICE_BREAK_WITHOUT_INTERFACE = 0,

    /**
     1 \n
     Wyst�pienie b��du nie daje komunikatu i nie zawiesza przetwarzania.
     Rodzaj b��du mo�e by� testowany przy u�yciu sekwencji ��danie odes�ania informacji kasowych (pole Ostatni b��d odpowiedzi),
     lub sekwencj� ��danie odes�ania kodu b��du ostatniego rozkazu
     */
     DEVICE_NO_BREAK = 1,

     /**
      2 \n
      Wyst�pienie b��du w trakcie realizacji sekwencji powoduje wy�wietlenie komunikatu i zatrzymanie przetwarzania
      a� do naci�ni�cia klawisza OK.<br>
      Kod b��du po wykonaniu rozkazu jest automatycznie wysy�any do interfejsu
      */
      DEVICE_BREAK_WITH_INTERFACE = 2,

     /**
      3 \n
      Wyst�pienie b��du nie daje komunikatu i nie zawiesza przetwarzania.\n
      Kod b��du po wykonaniu rozkazu jest automatycznie wysy�any do interfejsu \n
      Przy tej obs�udze nale�y po wysy�ce odebra� dane z urz�dzenia,
      chyba, �e opis protoko�u danej komendy m�wi inaczej.
     */
      DEVICE_NO_BREAK_WITH_INTERFACE = 3
};