#pragma once
/*! \file */
/**
\brief Tryby obs³ugi b³êdów
\ingroup Enumerations
*/
enum ErrorMode : size_t {
    /**
     0 \n
     Wyst¹pienie b³êdu w trakcie realizacji sekwencji powoduje wyœwietlenie komunikatu i zatrzymanie przetwarzania,
     a¿ do naciœniêcia klawisza OK \n
     Jest to domyœlny tryb po w³¹czeniu urz¹dzenia.
     */
    DEVICE_BREAK_WITHOUT_INTERFACE = 0,

    /**
     1 \n
     Wyst¹pienie b³êdu nie daje komunikatu i nie zawiesza przetwarzania.
     Rodzaj b³êdu mo¿e byæ testowany przy u¿yciu sekwencji ¿¹danie odes³ania informacji kasowych (pole Ostatni b³¹d odpowiedzi),
     lub sekwencj¹ ¿¹danie odes³ania kodu b³êdu ostatniego rozkazu
     */
     DEVICE_NO_BREAK = 1,

     /**
      2 \n
      Wyst¹pienie b³êdu w trakcie realizacji sekwencji powoduje wyœwietlenie komunikatu i zatrzymanie przetwarzania
      a¿ do naciœniêcia klawisza OK.<br>
      Kod b³êdu po wykonaniu rozkazu jest automatycznie wysy³any do interfejsu
      */
      DEVICE_BREAK_WITH_INTERFACE = 2,

     /**
      3 \n
      Wyst¹pienie b³êdu nie daje komunikatu i nie zawiesza przetwarzania.\n
      Kod b³êdu po wykonaniu rozkazu jest automatycznie wysy³any do interfejsu \n
      Przy tej obs³udze nale¿y po wysy³ce odebraæ dane z urz¹dzenia,
      chyba, ¿e opis protoko³u danej komendy mówi inaczej.
     */
      DEVICE_NO_BREAK_WITH_INTERFACE = 3
};