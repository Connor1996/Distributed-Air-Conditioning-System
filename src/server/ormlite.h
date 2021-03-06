﻿#ifndef ORMLITE_H
#define ORMLITE_H

#include "../include/sqlite3.h"

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <thread>

// 注入宏，存储元信息
#define ORMAP(_CLASS_NAME_, ...)                                \
private:                                                        \
friend class ORMLite::ORMapper<_CLASS_NAME_>;                   \
friend class ORMLite::QueryMessager<_CLASS_NAME_>;              \
template <typename VISITOR>                                     \
void _Accept(VISITOR& visitor) const                            \
{                                                               \
    visitor.Visit(__VA_ARGS__);                                 \
}                                                               \
constexpr static const char* _CLASSNAME = #_CLASS_NAME_;        \
constexpr static const char* _FIELDSNAME = #__VA_ARGS__

namespace ORMLite{

// Nullable类型模版，允许变量值为nullptr
template <typename T>
class Nullable
{
    template <typename T2>
    friend bool operator== (const Nullable<T2> &op1,
                            const Nullable<T2> &op2);
    template <typename T2>
    friend bool operator== (const Nullable<T2> &op,
                            const T2 &value);
    template <typename T2>
    friend bool operator== (const T2 &value,
                            const Nullable<T2> &op);
    template <typename T2>
    friend bool operator== (const Nullable<T2> &op,
                            std::nullptr_t);
    template <typename T2>
    friend bool operator== (std::nullptr_t,
                            const Nullable<T2> &op);
public:
    // Default or Null Construction
    Nullable ()
        : m_hasValue (false), m_value (T ()) {}
    Nullable (std::nullptr_t)
        : Nullable () {}

    // Null Assignment
    const Nullable<T> & operator= (std::nullptr_t)
    {
        m_hasValue = false;
        m_value = T ();
        return *this;
    }

    // Value Construction
    Nullable (const T &value)
        : m_hasValue (true), m_value (value) {}

    // Value Assignment
    const Nullable<T> & operator= (const T &value)
    {
        m_hasValue = true;
        m_value = value;
        return *this;
    }

private:
    bool m_hasValue;
    T m_value;

public:
    const T &Value () const
    { return m_value; }
};

    // == varialbe
    template <typename T2>
    inline bool operator== (const Nullable<T2> &op1,
                            const Nullable<T2> &op2)
    {
        return op1.m_hasValue == op2.m_hasValue &&
            (!op1.m_hasValue || op1.m_value == op2.m_value);
    }

    // == value
    template <typename T2>
    inline bool operator== (const Nullable<T2> &op,
                            const T2 &value)
    {
        return op.m_hasValue && op.m_value == value;
    }
    template <typename T2>
    inline bool operator== (const T2 &value,
                            const Nullable<T2> &op)
    {
        return op.m_hasValue && op.m_value == value;
    }

    // == nullptr
    template <typename T2>
    inline bool operator== (const Nullable<T2> &op,
                            std::nullptr_t)
    {
        return !op.m_hasValue;
    }
    template <typename T2>
    inline bool operator== (std::nullptr_t,
                            const Nullable<T2> &op)
    {
        return !op.m_hasValue;
    }
}

namespace ORMLite_Impl {

// 管理与SQLite3的连接
class SQLConnector
{
public:
    SQLConnector(const std::string& fileName)
    {
        if (sqlite3_open(fileName.c_str(), &_database))
        {
            sqlite3_close(_database);
            throw std::runtime_error(
                        std::string("Can't open database: %s\n") +
                                    sqlite3_errmsg(_database));
        }
    }

    ~SQLConnector()
    {
        sqlite3_close(_database);
    }



    void Execute(const std::string &sqlStr,
                 std::function<void (int, char**, char**)> callback = _callback)
    {
        const size_t MAX_TRIAL = 6;
        char *errMsg = 0;
        int result;

        // 回调函数包装器
        auto callbackWrapper = [] (void* fn, int column,
                char** columnText, char** columnName) -> int
        {
            static_cast<std::function<void (int, char**, char**)> *>
                    (fn)->operator()(column, columnText, columnName);
            return 0;
        };

        // 若数据库繁忙，等待后重试
        for (size_t i = 0; i < MAX_TRIAL; i++)
        {
            result = sqlite3_exec(_database, sqlStr.c_str(), callbackWrapper, &callback, &errMsg);
            if (result != SQLITE_BUSY)
                break;

            std::this_thread::sleep_for(std::chrono::microseconds(20));
        }

        if (result != SQLITE_OK)
        {
            auto errStr = "SQL Error: " + std::string(errMsg);

            // errMsg由sqlite3_malloc()产生，所以需要手动sqlite3_free()
            sqlite3_free(errMsg);
            throw std::runtime_error(errStr);
        }


    }

private:
    sqlite3* _database;

    static void _callback(int column, char** columnText, char** columnName)
    {
        return;
    }
};

// Field表达式转换成pair
struct Expr
{
    std::vector<std::pair<const void *, std::string>> expr;


    template <typename T>
    Expr (const ORMLite::Nullable<T>& property, const std::string& op,
          const ORMLite::Nullable<T>& value)
        : expr{ std::make_pair(&property, op + std::to_string(value.Value())) }
    { }

    template <typename T>
    Expr (const T& property, const std::string& op, const T& value)
        : expr{ std::make_pair(&property, op + std::to_string(value)) }
    { }



    template <>
    Expr (const std::string& property, const std::string& op,
         const std::string& value)
        : expr{ std::make_pair(&property, op + "'" + value + "'") }
    { }

    inline Expr operator &&(const Expr& rhs)
    {
        return _OperatorToString(rhs, " AND ");
    }

    inline Expr operator ||(const Expr& rhs)
    {
        return _OperatorToString(rhs, " OR ");
    }

private:
    Expr _OperatorToString(const Expr& rhs, std::string op)
    {
        // 前面加括号，最后面也要加括号，以确保运算顺序
        expr.emplace(expr.begin(), std::make_pair(nullptr, "("));
        expr.emplace_back(std::make_pair(nullptr, std::move(op)));
        for (const auto exprPair : rhs.expr)
            expr.emplace_back(exprPair);
        expr.emplace_back(std::make_pair(nullptr, ")"));

        return *this;
    }
};

// 每个属性都由Field()修饰，即创建Field_Expr对象
// 重载Field_Expr的操作符，使得布尔表达式转换成自定义的Expr对象
// 以满足生成相应的sql语句
template <typename T>
struct Field_Expr
{
    const T& _property;

    Field_Expr(const T& property)
        : _property(property)
    {}

    inline Expr operator ==(T value)
    {
        return Expr{_property, "=", std::move(value)};
    }

    inline Expr operator !=(T value)
    {
        return Expr{_property, "!=", std::move(value)};
    }

    inline Expr operator >(T value)
    {
        return Expr{_property, ">", std::move(value)};
    }

    inline Expr operator <(T value)
    {
        return Expr{_property, "<", std::move(value)};
    }

    inline Expr operator >=(T value)
    {
        return Expr{_property, ">=", std::move(value)};
    }

    inline Expr operator <=(T value)
    {
        return Expr{_property, "<=", std::move(value)};
    }

};

// 类型访问者
class TypeVisitor
{
public:

    template <typename ...Args>
    inline void Visit(Args&  ...args)
    {
        _Visit(args...);
    }

    std::string GetSerializedStr() const { return _serializedStr; }

protected:
    std::string _serializedStr;

    template <typename First, typename ...Args>
    inline void _Visit(First& property, Args&  ...args)
    {
        _Visit(property);
        _Visit(args...);
    }

    inline void _Visit(const int& property)
    {
        _serializedStr += "INT";
        _serializedStr += ",";
    }
    inline void _Visit(const double& property)
    {
        _serializedStr += "REAL";
        _serializedStr += ",";
    }
    inline void _Visit(const std::string& property)
    {
        _serializedStr += "TEXT";
        _serializedStr += ",";
    }
};

// 读取访问者
class ReaderVisitor
{
public:
    template <typename ...Args>
    inline void Visit(Args&  ...args)
    {
        return _Visit(args...);
    }

    std::string GetSerializedStr() const { return _serializedStr; }

protected:
    std::string _serializedStr;

    template <typename First, typename ...Args>
    inline void _Visit(First& property, Args& ...args)
    {
        _Visit(property);
        _Visit(args...);
    }


    inline void _Visit(const int& property)
    {
        _serializedStr += std::to_string(property) + ",";
    }
    inline void _Visit(const double& property)
    {
        _serializedStr += std::to_string(property) + ",";
    }
    inline void _Visit(const std::string& property)
    {
        _serializedStr += "'" + property + "'" + ",";
    }
    template <typename T>
    inline void _Visit(const ORMLite::Nullable<T> & property)
    {
        if (property == nullptr)
            _serializedStr += "NULL,";
        else
            _Visit(property.Value());
    }


};

// 写数据访问者
class WriterVisitor
{
public:
    WriterVisitor (std::string serializedValues)
        : _serializedValues (serializedValues)
    {}

    template <typename ...Args>
    inline void Visit(Args& ...args)
    {
        return _Visit(args...);
    }

private:
    std::string _serializedValues;

protected:
    template <typename First, typename ...Args>
    inline void _Visit(First& property, Args& ...args)
    {
        _Visit(property);
        _Visit(args...);
    }

    template <typename T>
    inline void _Visit (T &property)
    {
        // Remarks:
        // GCC Hell: explicit specialization in non-namespace scope
        // So, unable to Write Impl Here
        DeserializeValue (property, SplitStr (_serializedValues));
    }
};

// 变量下标访问者
class IndexVisitor
{
public:
    size_t index;
    bool isFound;

    IndexVisitor(const void* pointer)
        : index(0), isFound(false), _pointer(pointer)
    { }

    template <typename ...Args>
    inline void Visit(Args& ...args)
    {
        return _Visit(args...);
    }

protected:
    const void* _pointer;

    template <typename First, typename ...Args>
    inline void _Visit(First& property, Args& ...args)
    {
        _Visit(property);
        if (!isFound)
            _Visit(args...);
    }

    template <typename T>
    inline void _Visit(const T &property)
    {
        if ((const void *)&property == _pointer)
            isFound = true;
        else
            index++;
    }

};



}


namespace ORMLite {

template <typename T>
class ORMapper;

// 查询时用到的类，保存有查询信息
template <typename T>
class QueryMessager
{
    friend bool ORMapper<T>::Query(QueryMessager<T>& messager);

public:
    QueryMessager(const T& ModelObject)
        : _pModelObject(&ModelObject), _sqlWhere(""), _sqlOrderBy(""), _sqlLimit("")
    { }

    QueryMessager& Where(const ORMLite_Impl::Expr& expr)
    {
        _sqlWhere = "WHERE ";
        for (const auto exprPair : expr.expr)
        {
            // first为nullptr说明为括号
            if (exprPair.first != nullptr)
                _sqlWhere += _GetFieldName(exprPair.first);
            _sqlWhere += exprPair.second;
        }

        return *this;
    }

    QueryMessager& OrderBy(const T& property, bool isDecreasing = false)
    {
        _sqlOrderBy = " ORDER BY " + _GetFieldName(property);
        if (isDecreasing)
            _sqlOrderBy += " DESC";

        return *this;
    }

    QueryMessager& Limit(size_t count, size_t offest)
    {
        _sqlLimit = " LIMIT " + std::to_string(count) +
                " OFFEST " + std::to_string(offest);

        return *this;
    }

    std::vector<std::vector<std::string>> &GetVector()
    {
        return _result;
    }

    std::vector<T> GetObjects()
    {
        // To do
    }

    bool IsNone()
    {
        return _result.size() == 0;
    }

    void Clear()
    {
        _sqlWhere.erase();
        _sqlOrderBy.erase();
        _sqlLimit.erase();
        _result.clear();
    }

    // 获取相应字段的下标
    std::string GetField(const std::string& key, const void* property)
    {
        ORMLite_Impl::IndexVisitor visitor(property);
        _pModelObject->_Accept(visitor);

        for (auto row : _result)
        {
            if (row[0] == key)
                return row[visitor.index];
        }

        return "";
    }

private:
    const T* _pModelObject;
    std::string _sqlWhere;
    std::string _sqlOrderBy;
    std::string _sqlLimit;

    std::vector<std::vector<std::string>> _result;

    std::string _GetFieldName(const void* property)
    {
        ORMLite_Impl::IndexVisitor visitor(property);
        _pModelObject->_Accept(visitor);

        if (!visitor.isFound)
            throw std::runtime_error("No such field in the Table");

        return _Split(T::_FIELDSNAME)[visitor.index];
    }

    std::vector<std::string> _Split(std::string str)
    {
        std::vector<std::string> result;
        std::string tempStr = "";
        // 扩展字符串方便操作
        str += ',';

        for (const auto& ch : str)
        {
            switch (ch)
            {
            case ',':
                result.push_back(tempStr);
                tempStr.clear();
                break;
            case ' ':
            case '_':
                break;
            default:
                tempStr += ch;
                break;
            }
        }
        return std::move(result);
    }

};



template <typename T>
class ORMapper
{
public:
    ORMapper(const std::string& dbName)
        : _dbName(dbName), _tableName(T::_CLASSNAME),
          _fieldsName(_Split(T::_FIELDSNAME))
    {
        CreateTable();
    }

    bool CreateTable()
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            // 创建model类，使用访问者访问获得类中元信息
            const T modelObject {};
            ORMLite_Impl::TypeVisitor visitor;
            modelObject._Accept(visitor);

            auto fieldsType = _Split(visitor.GetSerializedStr());

            auto fieldsStr = _fieldsName[0] + " " +
                    fieldsType[0] + " PRIMARY KEY NOT NULL,";

            for(auto i = 1; i < _fieldsName.size(); i++)
            {
                fieldsStr += _fieldsName[i] + " " +
                        fieldsType[i] + ",";
            }
            // 去除字符串最后多余的逗号
            fieldsStr.pop_back();

            connector.Execute("CREATE TABLE IF NOT EXISTS " + _tableName +
                              + "(" + std::move(fieldsStr) + ");");
        });
    }

    bool DropTable()
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            connector.Execute("DROP TABLE " + _tableName + ";");
        });
    }

    bool Insert(const T& modelObject)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            ORMLite_Impl::ReaderVisitor visitor;
            modelObject._Accept(visitor);

            auto fieldsStr = visitor.GetSerializedStr();
            fieldsStr.pop_back();

            connector.Execute("BEGIN TRANSACTION;" \
                              " INSERT INTO " + _tableName +
                              " VALUES (" + fieldsStr + ");" +
                              " COMMIT TRANSACTION;");
        });
    }

    // 读入指定的对象，根据该对象的主键删除整行
    bool Delete(const T& modelObject)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            ORMLite_Impl::ReaderVisitor visitor;
            modelObject._Accept(visitor);

            // 格式如 "property = value"
            auto keyValueStr = _fieldsName[0] + "=" + _Split(visitor.GetSerializedStr())[0];
            connector.Execute("DELETE FROM " + _tableName + " " +
                             "WHERE " + std::move(keyValueStr) + ";");
        });
    }

    // 删除满足propety=value的行
    bool Delete(const std::string& property, const std::string& value)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            connector.Execute("DELETE FROM " + _tableName + " " +
                              "WHERE " + property + "=" + value + ";");
        });
    }

    // 删除所有行
    bool DeleteAll()
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            connector.Execute("DELETE FROM " + _tableName + ";");
        });
    }

    // 根据指定的对象，更新相应的行
    bool Update(const T& modelObject)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            ORMLite_Impl::ReaderVisitor visitor;
            modelObject._Accept(visitor);

            auto fieldsValue = _Split(visitor.GetSerializedStr());
            auto keyStr = _fieldsName[0] + "=" + fieldsValue[0];
            std::string fieldsStr = "";
            for(auto i = 1; i < _fieldsName.size(); i++)
            {
                fieldsStr += _fieldsName[i] + "=" +
                        fieldsValue[i] + ",";
            }
            fieldsStr.pop_back();

            connector.Execute("UPDATE " + _tableName +
                              " SET " + std::move(fieldsStr) +
                              " WHERE " + std::move(keyStr) + ";");
        });
    }

    // 设置key=keyValue那行的property为value
    bool Update(const std::string& keyValue, const std::string& property,
                const std::string& value)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            const T object {};
            ORMLite_Impl::ReaderVisitor visitor;
            object._Accept(visitor);

            auto key = _Split(visitor.GetSerializedStr())[0];

            connector.Execute("UPDATE " + _tableName +
                              " SET " + property + "=" + value +
                              " WHERE " + std::move(key) + "=" + keyValue + ";");
        });
    }


    // 查询返回的是由相应数据初始化的类型对象
    bool Query(QueryMessager<T>& messager)
    {
        return _HandleException([&] (ORMLite_Impl::SQLConnector& connector)
        {
            std::string sqlStr = "SELECT * FROM " + _tableName + " ";
            if (!messager._sqlWhere.empty())
                sqlStr += messager._sqlWhere;
            if (!messager._sqlLimit.empty())
                sqlStr += messager._sqlOrderBy;
            if (!messager._sqlOrderBy.empty())
                sqlStr += messager._sqlLimit;

            connector.Execute(sqlStr + ";", [&] (int column, char** columnText, char
                              **columnName)
            {
               // 每次sqlite查询一行后都会调用一次callback函数
               // 故callback里面每次只记录一个row
               std::vector<std::string> row;
               for (int i = 0; i < column; i++)
                   row.push_back(columnText[i]);

               messager._result.push_back(std::move(row));
            });
        });
    }

    std::string GetErrorMessage()
    {
        return _errorMessage;
    }


private:
    const std::string _dbName;
    const std::string _tableName;
    const std::vector<std::string> _fieldsName;
    std::string _errorMessage;

    bool _HandleException(std::function<void (ORMLite_Impl::SQLConnector& )> fn)
    {
        try
        {
            ORMLite_Impl::SQLConnector connector(_dbName);
            fn(connector);
            return true;
        }
        catch (const std::exception& e)
        {
            _errorMessage = e.what();
            std::cout << _errorMessage << std::endl;
            return false;
        }
    }

    std::vector<std::string> _Split(std::string str)
    {
        std::vector<std::string> result;
        std::string tempStr = "";
        // 扩展字符串方便操作
        str += ',';

        for (const auto& ch : str)
        {
            switch (ch)
            {
            case ',':
                result.push_back(tempStr);
                tempStr.clear();
                break;
            case ' ':
            case '_':
                break;
            default:
                tempStr += ch;
                break;
            }
        }
        return std::move(result);
    }

};


template <typename T>
inline ORMLite_Impl::Field_Expr<T> Field(T& property)
{
    return ORMLite_Impl::Field_Expr<T>(property);
}

template <typename T>
inline ORMLite_Impl::Field_Expr<T> Field(T&& property) = delete;

}



#endif // ORMLITE_H
