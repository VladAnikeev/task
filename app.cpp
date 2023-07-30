#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <ctime>
#include "log.h"

//! --------------------------------- ГЛОБАЛЬНЫЙ ПЕРЕМЕННЫЕ  -------------------------------

Log logging;

//! --------------------------------- ---------------------  -------------------------------

/**
 * @brief преобразует в тив wstring
 *
 * @tparam T любой тип
 * @param val значение
 * @return возвращает преобразованый в std::wstring
 */
template <typename T>
std::wstring toWstring(T val)
{
    std::wostringstream oss;
    oss << val;
    return oss.str();
}

/**
 * @brief из файла в текст
 *
 * @param filename путь файла
 * @return текст файла в формате std::wstring
 */
std::wstring readFile(const char *filename)
{
    if (!filename)
    {
        logging << "#ERROR there is no required command line argument\n";
        throw "";
        return L"";
    }
    std::wifstream file(filename);
    if (!file.is_open())
    {
        logging << "#ERROR failed file opening: " << filename << "\n";
        throw "";
        return L"";
    }

    file.imbue(std::locale("rus_rus.866"));

    std::wstringstream buf;
    buf.imbue(std::locale("rus_rus.866"));

    buf << file.rdbuf();

    if (!(buf.str().length()))
    {
        logging << "#ERROR the file is empty: " << filename << "\n";
        throw "";
        return L"";
    }

    return buf.str();
}

/**
 * @brief функция для замены строк
 *
 * @param inputStr где заменить
 * @param src что заменить
 * @param dst на что заменить
 * @return std::wstring - результат
 */
std::wstring stringReplacer(std::wstring &inputStr, std::wstring src, std::wstring dst)
{
    std::wstring result(inputStr);

    size_t pos = result.find(src);
    while (pos != std::wstring::npos)
    {
        result.replace(pos, src.size(), dst);
        pos = result.find(src, pos);
    }
    return result;
}

/**
 * @brief находит по двум символом и вырезает
 *
 * @param inputStr в чм искать и вырезать
 * @param symbol_begin начальный символ
 * @param symbol_end конечный символ
 * @return std::wstring повращает найденное
 */
std::wstring find_and_cut(std::wstring &inputStr, const wchar_t *symbol_begin, const wchar_t *symbol_end)
{
    // если нет символа, вернет -1
    // лучше int, чем size_t - только натуральные числа

    int first = inputStr.find(symbol_begin); // начальная позиция символа

    if (first == -1)
    {
        return L"";
    }
    first++;

    int second = inputStr.find(symbol_end, first); // конечная позиция символа

    if (second == -1)
    {
        return L"";
    }
    // запоминаем как ключ
    std::wstring result = inputStr.substr(first, second - first);

    inputStr = inputStr.erase(first - 1, result.length() + 2);
    return result;
}

/**
 * @brief возвращает текущую дату в формате DD.MM.YY
 *
 * @return std::wstring
 */
std::wstring currentDate()
{
    // создание времени по C
    struct tm now_date;
    time_t now = time(0);
    localtime_s(&now_date, &now);

    // конвертация в unicode
    std::wstring buf;
    buf += toWstring(now_date.tm_mday) + L':';

    now_date.tm_mon += 1;
    if (now_date.tm_mon < 10)
    {
        buf += L"0";
    }
    buf += toWstring(now_date.tm_mon) + L':' + toWstring(now_date.tm_year + 1900);

    return buf;
}

int main(int argc, char *argv[])
{
    try
    {
        //!     Открытие JSON

        logging << "#INF opening a file JSON \n";
        std::wstring file_json = readFile(argv[1]);

        //!     Парсинг JSON

        // поток для записи
        std::map<std::wstring, std::wstring> json;

        logging << "#INF parsing JSON\n";
        // цикл пока не будет найден символ
        while (file_json.find('\"') != -1)
        {
            std::wstring key = find_and_cut(file_json, L"\"", L"\"");
            std::wstring value = find_and_cut(file_json, L"\"", L"\"");

            json[key] = value;
        }

        //!     Чтения шаблона

        logging << "#INF opening a file pattern \n";
        std::wstring input_pattern = readFile(argv[2]);

        //!     Поиск важных и удаление *

        logging << "#INF search for important \'*\' \n";
        while (input_pattern.find('*') != -1)
        {

            size_t first = input_pattern.find(L"*") + 1;     // начальная позиция символа
            size_t second = input_pattern.find(L"}", first); // конечная позиция символа

            // запоминаем как ключ
            std::wstring key = input_pattern.substr(first, second - first);

            // удаляем *
            input_pattern.erase(first - 1, 1);

            // поиск обязательного
            std::map<std::wstring, std::wstring>::iterator it_json = json.find(key);
            if (it_json == json.end())
            {
                logging << "#ERROR required template parameter is missing\n";
            }
        }

        //!     Вставка даты
        logging << "#INF date rate\n";

        json[L"currentDate"] = currentDate();
        input_pattern = stringReplacer(input_pattern, L"{currentDate}", currentDate());

        //!      Замена данных в шаблоне

        for (std::map<std::wstring, std::wstring>::iterator it_json = json.begin(); it_json != json.end(); it_json++)
        {
            input_pattern = stringReplacer(input_pattern, L"{" + it_json->first + L"}", it_json->second);
        }

        //!     Удаляем не найденные в json и необязательные

        logging << "#INF removing unnecessary parameters\n";
        find_and_cut(input_pattern, L"{", L"}");

        if (!argv[3])
        {
            logging << "#ERROR there is no required command line argument\n";
            throw "";
        }
        //!     Сохранение результата

        logging << "#INF saving the result\n";
        std::wofstream out(argv[3]);
        out.imbue(std::locale("rus_rus.866"));

        out << input_pattern;
    }
    catch (const char *error_message)
    {
        return -1;
    }
    return 0;
}
