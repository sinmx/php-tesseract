#pragma once

extern "C" {
#include <php.h>
#include <Zend/zend_interfaces.h>
}

#include <new>
#include "src/tesseract.h"
#include "src/future.h"
#include <thread>

namespace {

    PHP_METHOD(Tesseract, fromFile)
    {
        char* file_path;
        size_t len;
        HashTable* langArray = NULL;
        zval* lang;
        std::string languages = "";

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|h", &file_path, &len, &langArray) == FAILURE) {
            return;
        }

        if (langArray) {
            bool firstLanguage = true;
            ZEND_HASH_FOREACH_VAL(langArray, lang) {
                if (Z_TYPE_P(lang) == IS_STRING) {
                    if (firstLanguage) {
                        firstLanguage = false;
                    } else {
                        languages.append("+");
                    }

                    languages.append(Z_STRVAL_P(lang));
                }
            } ZEND_HASH_FOREACH_END();
        } else {
            languages = "eng";
        }

        Pix *image = pixRead(file_path);

        object_init_ex(return_value, tesseract_ce);
        auto intern = Z_OBJECT_TESSERACT(Z_OBJ_P(return_value));
        new (intern) tesseract::php::Tesseract(image, languages);
    }

    PHP_METHOD(Tesseract, fromString)
    {
        char* file_content;
        size_t len;
        HashTable* langArray = NULL;
        zval* lang;
        std::string languages = "";

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|h", &file_content, &len, &langArray) == FAILURE) {
            return;
        }

        if (langArray) {
            bool firstLanguage = true;
            ZEND_HASH_FOREACH_VAL(langArray, lang) {
                if (Z_TYPE_P(lang) == IS_STRING) {
                    if (firstLanguage) {
                        firstLanguage = false;
                    } else {
                        languages.append("+");
                    }

                    languages.append(Z_STRVAL_P(lang));
                }
            } ZEND_HASH_FOREACH_END();
        } else {
            languages = "eng";
        }

        Pix* image = pixReadMem(reinterpret_cast<l_uint8*>(file_content), len);

        object_init_ex(return_value, tesseract_ce);
        auto intern = Z_OBJECT_TESSERACT(Z_OBJ_P(return_value));
        new (intern) tesseract::php::Tesseract(image, languages);
    }

    PHP_METHOD(Tesseract, getText)
    {
        if (zend_parse_parameters_none() == FAILURE) {
            return;
        }

        auto intern = Z_OBJECT_TESSERACT_P(getThis());

        RETURN_STRING(intern->get_text());
    }

    PHP_METHOD(Tesseract, getTextAsync)
    {
        if (zend_parse_parameters_none() == FAILURE) {
            return;
        }

        auto intern = Z_OBJECT_TESSERACT_P(getThis());

        std::shared_future<char*> future = intern->get_text_async().share();

        object_init_ex(return_value, future_ce);
        auto internFuture = Z_OBJECT_FUTURE(Z_OBJ_P(return_value));
        new (internFuture) tesseract::php::Future(std::move(future));
    }

    PHP_METHOD(Tesseract, getHocrText)
    {
        if (zend_parse_parameters_none() == FAILURE) {
            return;
        }

        auto intern = Z_OBJECT_TESSERACT_P(getThis());

        RETURN_STRING(intern->get_hocr_text());
    }

    PHP_METHOD(Tesseract, getRectangle)
    {
        long left, top, width, height;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &left, &top, &width, &height) == FAILURE) {
            return;
        }

        auto intern = Z_OBJECT_TESSERACT_P(getThis());
        intern->set_rectangle(left, top, width, height);

        RETURN_ZVAL(getThis(), 1, 0);
    }

    PHP_METHOD(Tesseract, setPageSegMode)
    {
        long mode;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE) {
            return;
        }

        auto intern = Z_OBJECT_TESSERACT_P(getThis());
        intern->set_page_seg_mode(mode);

        RETURN_ZVAL(getThis(), 1, 0);
    }

    ZEND_BEGIN_ARG_INFO_EX(tesseract_tesseract_void, 0, 0, 0)
    ZEND_END_ARG_INFO()

    ZEND_BEGIN_ARG_INFO_EX(tesseract_tesseract_from_file, 0, 0, 0)
        ZEND_ARG_INFO(0, file_path)
    ZEND_END_ARG_INFO()

    ZEND_BEGIN_ARG_INFO_EX(tesseract_tesseract_from_string, 0, 0, 0)
        ZEND_ARG_INFO(0, file_content)
    ZEND_END_ARG_INFO()

    ZEND_BEGIN_ARG_INFO_EX(tesseract_tesseract_get_rectangle, 0, 0, 0)
        ZEND_ARG_INFO(0, left)
        ZEND_ARG_INFO(0, top)
        ZEND_ARG_INFO(0, width)
        ZEND_ARG_INFO(0, height)
    ZEND_END_ARG_INFO()

    ZEND_BEGIN_ARG_INFO_EX(tesseract_tesseract_set_page_seg_mode, 0, 0, 0)
        ZEND_ARG_INFO(0, mode)
    ZEND_END_ARG_INFO()

    zend_function_entry tesseract_methods[] = {
        PHP_ME(Tesseract, fromFile, tesseract_tesseract_from_file, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(Tesseract, fromString, tesseract_tesseract_from_string, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(Tesseract, getText, tesseract_tesseract_void, ZEND_ACC_PUBLIC)
        PHP_ME(Tesseract, getTextAsync, tesseract_tesseract_void, ZEND_ACC_PUBLIC)
        PHP_ME(Tesseract, getHocrText, tesseract_tesseract_void, ZEND_ACC_PUBLIC)
        PHP_ME(Tesseract, getRectangle, tesseract_tesseract_get_rectangle, ZEND_ACC_PUBLIC)
        PHP_ME(Tesseract, setPageSegMode, tesseract_tesseract_set_page_seg_mode, ZEND_ACC_PUBLIC)
        PHP_FE_END
    };

    void init_tesseract_ce()
    {
        zend_class_entry ce;

        INIT_CLASS_ENTRY(ce, "Tesseract\\Tesseract", tesseract_methods);
        tesseract_ce = zend_register_internal_class(&ce TSRMLS_CC);
        tesseract_ce->create_object = tesseract::php::Tesseract::create_object;

        memcpy(&tesseract::php::Tesseract::handler_tesseract, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
        tesseract::php::Tesseract::handler_tesseract.offset = XtOffsetOf(tesseract::php::Tesseract, std);
    }
}