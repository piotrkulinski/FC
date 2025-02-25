#pragma once
/**
\ingroup Enumerations
\brief Standardowe protoko�y, kt�re mog� by� zaimplementowane w urz�dzeniach
*/
enum class DeviceProtocol : size_t {
	
	Unknow,
	/**
	 * 0\n
	 * Najcz�ciej implementowany protok� w urz�dzeniach fiskalnych.
	 */
	Thermal,

	/**
	 * 1\n
	 * Wy��cznie w urz�dzeniach Posnet\n
	 * Implementowany jako standard, zast�pi� protok� thermal.
	 */
	Posnet,

	/**
	 * 2\n
	 * Wy��cznie w urz�dzeniach Novitus.\n
	 * Dodatkowo w urz�dzeniach, mo�na czasem wybra� protok� <b>NOVITUS ZGOD.</b>, kt�ry odpowiada protoko�owi thermal.
	 */
	Novitus,

	/**
	 * 3\n
	 * Wy��cznie w urz�dzeniach Novitus.\n
	 * Nie u�ywany w aktualnej bibliotece.
	 */
	NovitusXML,

	/**
	 * 4\n
	 * Nie u�ywany w aktualnej bibliotece.
	 */
	STX,

	/**
	 * 5\n
	 * Wy��cznie w urz�dzeniach Elzab.\n
	 */
	ElzabPOS,

	/**
	 * @brief Obs�uga z wykorzystaniem DLL
	*/
	ElzabDLL
};
