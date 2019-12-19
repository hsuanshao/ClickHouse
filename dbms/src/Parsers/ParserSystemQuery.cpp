#include <Parsers/ParserSystemQuery.h>
#include <Parsers/ASTSystemQuery.h>
#include <Parsers/CommonParsers.h>
#include <Parsers/parseIdentifierOrStringLiteral.h>
#include <Parsers/parseDatabaseAndTableName.h>


namespace ErrorCodes
{
    extern const int NOT_IMPLEMENTED;
}


namespace DB
{


bool ParserSystemQuery::parseImpl(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword{"SYSTEM"}.ignore(pos, expected))
        return false;

    using Type = ASTSystemQuery::Type;

    auto res = std::make_shared<ASTSystemQuery>();

    bool found = false;
    for (int i = static_cast<int>(Type::UNKNOWN) + 1; i < static_cast<int>(Type::END); ++i)
    {
        Type t = static_cast<Type>(i);
        if (ParserKeyword{ASTSystemQuery::typeToString(t)}.ignore(pos, expected))
        {
            res->type = t;
            found = true;
        }
    }

    if (!found)
        return false;

    switch (res->type)
    {
        case Type::RELOAD_DICTIONARY:
            if (ParserKeyword{"ON"}.ignore(pos, expected))
            {
                if (!ASTQueryWithOnCluster::parse(pos, res->cluster, expected))
                    return false;
            }
            if (!parseIdentifierOrStringLiteral(pos, expected, res->target_dictionary))
                return false;
            break;

        case Type::RESTART_REPLICA:
        case Type::SYNC_REPLICA:
        case Type::FLUSH_DISTRIBUTED:
            if (!parseDatabaseAndTableName(pos, expected, res->database, res->target_table))
                return false;
            break;

        case Type::STOP_MERGES:
        case Type::START_MERGES:
        case Type::STOP_TTL_MERGES:
        case Type::START_TTL_MERGES:
        case Type::STOP_MOVES:
        case Type::START_MOVES:
        case Type::STOP_FETCHES:
        case Type::START_FETCHES:
        case Type::STOP_REPLICATED_SENDS:
        case Type::START_REPLICATED_SENDS:
        case Type::STOP_REPLICATION_QUEUES:
        case Type::START_REPLICATION_QUEUES:
        case Type::STOP_DISTRIBUTED_SENDS:
        case Type::START_DISTRIBUTED_SENDS:
            parseDatabaseAndTableName(pos, expected, res->database, res->target_table);
            break;

        default:
            /// There are no [db.table] after COMMAND NAME
            break;
    }

    node = std::move(res);
    return true;
}

}
