#include <iostream>
#include <string>
#include <map>

using namespace std;

// ������������ ��� ��� ������� ������
enum class TaskStatus
{
     NEW,          // �����
     IN_PROGRESS,  // � ����������
     TESTING,      // �� ������������
     DONE          // ���������
};

// ��������� ���-������� ��� map<TaskStatus, int>,
// ������������ ������� ���������� ����� ������� �������
using TasksInfo = map< TaskStatus, int >;

class TeamTasks
{
public:
     // �������� ���������� �� �������� ����� ����������� ������������
     const TasksInfo& GetPersonTasksInfo( const string& person ) const
     {
          return storage_.at( person );
     }

     // �������� ����� ������ (� ������� NEW) ��� ����������� �������������
     void AddNewTask( const string& person )
     {
          storage_[ person ][ TaskStatus::NEW ]++;
     }

     // �������� ������� �� ������� ���������� ����� ����������� ������������,
     // ����������� ��. ����
     tuple< TasksInfo, TasksInfo > PerformPersonTasks( const string& person, int task_count )
     {
          TasksInfo& currentTasks = storage_[ person ];
          TasksInfo newTasks;

          for( TaskStatus status = TaskStatus::NEW; status != TaskStatus::DONE; status = Next( status ) )
          {
               if( task_count == 0 )
               {
                    break;
               }

               while( currentTasks[ status ] > 0 && task_count != 0 )
               {
                    currentTasks[ status ]--;
                    newTasks[ Next( status ) ]++;
                    task_count--;
               }
          }

          TasksInfo oldTasks = currentTasks;

          for( const auto& [ key, value ]: newTasks )
          {
               currentTasks[ key ] += value;
          }


          oldTasks.erase( TaskStatus::DONE );

          RemoveZeros( oldTasks );
          RemoveZeros( newTasks );

          return { newTasks, oldTasks };
     }

private:
     static void RemoveZeros( TasksInfo& teamTasks )
     {
          for( auto it = teamTasks.begin(); it != teamTasks.end(); )
          {
               if( it->second == 0 )
               {
                    it = teamTasks.erase( it );
               }
               else
               {
                    ++it;
               }
          }
     }


     static TaskStatus Next( TaskStatus status )
     {
          return static_cast< TaskStatus >( static_cast< int >( status ) + 1 );
     }

private:
     std::map< std::string, TasksInfo > storage_;
};

// ��������� ������� �� ��������, ����� ����� �����������
// ���������� � ������������� ������ � ������� [] � �������� 0,
// �� ����� ��� ���� �������� �������
void PrintTasksInfo( TasksInfo tasks_info )
{
     cout << tasks_info[ TaskStatus::NEW ] << " new tasks" <<
          ", " << tasks_info[ TaskStatus::IN_PROGRESS ] << " tasks in progress" <<
          ", " << tasks_info[ TaskStatus::TESTING ] << " tasks are being tested" <<
          ", " << tasks_info[ TaskStatus::DONE ] << " tasks are done" << endl;
}

int main()
{
     TeamTasks tasks;
     tasks.AddNewTask( "Ilia" );
     for( int i = 0; i < 3; ++i )
     {
          tasks.AddNewTask( "Ivan" );
     }
     cout << "Ilia's tasks: ";
     PrintTasksInfo( tasks.GetPersonTasksInfo( "Ilia" ) );
     cout << "Ivan's tasks: ";
     PrintTasksInfo( tasks.GetPersonTasksInfo( "Ivan" ) );

     TasksInfo updated_tasks, untouched_tasks;

     tie( updated_tasks, untouched_tasks ) =
               tasks.PerformPersonTasks( "Ivan", 2 );
     cout << "Updated Ivan's tasks: ";
     PrintTasksInfo( updated_tasks );
     cout << "Untouched Ivan's tasks: ";
     PrintTasksInfo( untouched_tasks );

     tie( updated_tasks, untouched_tasks ) =
               tasks.PerformPersonTasks( "Ivan", 2 );
     cout << "Updated Ivan's tasks: ";
     PrintTasksInfo( updated_tasks );
     cout << "Untouched Ivan's tasks: ";
     PrintTasksInfo( untouched_tasks );

     return 0;
}
