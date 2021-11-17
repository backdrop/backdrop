<?php
/**
 * @file
 * Exceptions classes for Entity Reference module.
 */

/**
 * @class Exception class used to throw error if the entity view renderer goes
 * into a potentially infinite loop.
 */
class EntityReferenceRecursiveRenderingException extends Exception {}
