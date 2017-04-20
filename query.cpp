#include "query.hpp"

Query::Query(QString normalized_statement, QString statement, QString user, QString database, uint duration)
{
    this->executions = 0;
    this->totalDuration = 0;
    this->setStatement(normalized_statement);
    this->setUser(user);
    this->setDatabase(database);
    this->addStatement(duration, statement);
}

void Query::setStatement(QString statement){
    this->statement = statement;
}

void Query::setUser(QString user){
    this->user = user;
}

void Query::setDatabase(QString database){
    this->database = database;
}

void Query::addStatement(uint duration, QString statement){
    totalDuration += duration;
    executions++;
    durations.append(duration);
    if(examples.size() < 25)
        examples.append(statement);
}

uint Query::getAverageDuration(){
    return this->totalDuration / this->executions;
}

QRegExp filter_digits("([^_a-zA-Z0-9])\\d+");
QRegExp filter_limit("LIMIT \\d+", Qt::CaseInsensitive);
QRegExp filter_operator("(!=|>=|<=|!=|<|>|=)\\s*(\\d+|E?'[^']*')");
QRegExp filter_in(" (in|IN)\\s+\\([^\\)]+\\)");
QRegExp newline_match("(FROM|WHERE|LIMIT|GROUP BY|ORDER BY|LEFT JOIN|RIGHT JOIN|LEFT OUTER JOIN|RIGHT OUTER JOIN|JOIN)", Qt::CaseInsensitive);
QRegExp tabnewline_match("(AND|OR)",  Qt::CaseInsensitive);

#include <QTextStream>
QTextStream out(stdout, QIODevice::WriteOnly);

QString Query::normalize(QString statement){
    statement = statement.simplified();

    int start_index = 0;
    do{
        start_index = statement.indexOf(" IN (", start_index);
        if(start_index != -1){
            start_index += 5;
            int stop_index = statement.indexOf(QString(")"), start_index);
            if(stop_index != -1){
                statement = statement.replace(start_index, (stop_index - start_index), "N");
            }else{
                statement = statement.replace(start_index, (statement.length() - start_index), "N)");
            }
        }
    }while(start_index != -1);

    statement = statement.replace(filter_limit, "LIMIT N");
    statement = statement.replace(filter_operator, " \\1 N");

    // A not completely identical version of the filter_digit replace, but
    // quite a bit faster
    //statement = statement.replace(filter_digits, "\\1N");
    start_index = -1;
    int i = 0;
    while(i < statement.size()){
        if('0' >= statement[i] && '9' <= statement[i]){
            if(start_index == -1){
                start_index = i;
            }
        }else if(start_index != -1){
            statement = statement.replace(start_index, i - start_index, "N");
            i -= start_index;
            start_index = -1;
        }
        i++;
    }

    statement = format(statement);
    return statement;
}

QString Query::format(QString statement){
    statement = statement.replace(", ", ",\n\t");
    statement = statement.replace(newline_match, "\n\\1");
    statement = statement.replace(tabnewline_match, "\n\t\\1");
    statement.squeeze();
    return statement;
}
