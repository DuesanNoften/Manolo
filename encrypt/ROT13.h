#ifndef PROYECTO_2_ROT13_H
#define PROYECTO_2_ROT13_H

/*
 * @file rot13.h
 * @brief Header file that declares the function for applying the ROT13 cipher.
 * 
 * This file contains the declaration of the applyROT13 function, which applies
 * the ROT13 cipher to a given message. ROT13 is a simple encryption technique
 * where each letter in the message is replaced by the letter 13 positions ahead
 * in the alphabet. By applying ROT13 twice, the original text is recovered.
 * 
 * @note This cipher is a specific case of the Caesar Cipher, but with a rotation
 *       of 13 characters.
 * 
 * @author Juan Rodriguez
 * @version 1.0
 * @date 2024-11-20
 */

/**
 * @brief Applies the ROT13 cipher to a message.
 *
 * This function takes a text message and applies the ROT13 cipher to each of its
 * alphabetic characters. Non-alphabetic characters (such as numbers or punctuation)
 * remain unchanged.
 * 
 * The ROT13 cipher is commonly used to obscure text without the need for a key, 
 * and it is reversible. This means that applying ROT13 twice on the same message 
 * will return the original message.
 * 
 * @param message A string representing the message to be encrypted. The string is
 *                modified directly.
 * 
 * @note The function directly modifies the input message, so the original message
 *       will be altered after calling the function.
 * 
 * @example
 * char text[] = "Hello, World!";
 * applyROT13(text);
 * printf("%s", text);  // Output: "Uryyb, Jbeyq!"
 */
void applyROT13(char *message);

#endif // PROYECTO_2_ROT13_H
