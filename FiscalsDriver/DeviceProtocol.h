#pragma once
/**
\ingroup Enumerations
\brief Standardowe protoko³y, które mog¹ byæ zaimplementowane w urz¹dzeniach
*/
enum class DeviceProtocol : size_t {
	
	Unknow,
	/**
	 * 0\n
	 * Najczêœciej implementowany protokó³ w urz¹dzeniach fiskalnych.
	 */
	Thermal,

	/**
	 * 1\n
	 * Wy³¹cznie w urz¹dzeniach Posnet\n
	 * Implementowany jako standard, zast¹pi³ protokó³ thermal.
	 */
	Posnet,

	/**
	 * 2\n
	 * Wy³¹cznie w urz¹dzeniach Novitus.\n
	 * Dodatkowo w urz¹dzeniach, mo¿na czasem wybraæ protokó³ <b>NOVITUS ZGOD.</b>, który odpowiada protoko³owi thermal.
	 */
	Novitus,

	/**
	 * 3\n
	 * Wy³¹cznie w urz¹dzeniach Novitus.\n
	 * Nie u¿ywany w aktualnej bibliotece.
	 */
	NovitusXML,

	/**
	 * 4\n
	 * Nie u¿ywany w aktualnej bibliotece.
	 */
	STX,

	/**
	 * 5\n
	 * Wy³¹cznie w urz¹dzeniach Elzab.\n
	 */
	ElzabPOS,

	/**
	 * @brief Obs³uga z wykorzystaniem DLL
	*/
	ElzabDLL
};
