#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <string.h>

typedef int Element;
typedef char CElement[25]; 

typedef struct time{
	Element month;
	Element day;
	Element hour;
} TIME;
// plan linked_list

typedef struct plan{
	CElement name;
	TIME start;
	TIME end;
	Element done;
	struct plan* link;
} Node;
Node *head = NULL;

void error(char* str){
	printf("%s\n", str);
	exit(1);
}

void init_list(){head = NULL;}
int is_empty(){return head == NULL;}
int size()
{
	Node *p;
	int count = 0;
	for(p=head; p!=NULL; p=p->link) count++;
	
	return count;
}

Node* get_entry(int pos)
{
	int i;
	Node *p = head;
	
	for(i=0; i<pos; i++, p=p->link)
		if(p == NULL) return NULL;
	return p;
}

void print_day(int n, Element gMonth, Element gDay, int x, int y){
	int j = 0;
	Node *p;
	
	for(p=head; p!=NULL; p=p->link){
		if(p->start.month > gMonth || p->end.month < gMonth || (p->start.month == gMonth && p->start.day > gDay) ||
		(p->end.month == gMonth && p->end.day < gDay))
			continue;
		gotoxy(x, y+j);
		printf("%s", p->name);
		j += 1;
		if(n == 1 && j == 5)
			return;
	}
}

void find_name(CElement e){
	Node *p;
	int n = 18, i = 1;
	for(p=head; p!=NULL; p=p->link){
		if(strstr(p->name, e) != NULL){
			gotoxy(11, n);
			printf("%d. %s %d월%d일%d시~%d월%d일%d시", i, p->name, p->start.month, p->start.day, p->start.hour,
				p->end.month, p->end.day, p->end.hour);
			n+=2;
			i++;
		}
	}
}

void print_list(char *msg)
{
	Node* p;
	
	printf("%s[%2d] = ", msg, size());
	for(p=head; p!=NULL; p=p->link){
		printf("%s ", p->name);
		printf("시작: %d월%d일%d시", p->start.month, p->start.day, p->start.hour);
		printf("마감: %d월%d일%d시\n", p->end.month, p->end.day, p->end.hour);
		printf("\n");
	}
	printf("\n");
}

void insert_next(Node* prev, Node* node)
{
	if(node != NULL){
		node->link = prev->link;
		prev->link = node;
	}
}

void insert(int pos, TIME start, TIME end, CElement name, Element done)
{
	Node *new_node, *prev;
	
	new_node = (Node*)malloc(sizeof(Node));
	new_node->start.month = start.month;
	new_node->start.day = start.day;
	new_node->start.hour = start.hour;
	new_node->end.month = end.month;
	new_node->end.day = end.day;
	new_node->end.hour = end.hour;
	strcpy(new_node->name, name);
	new_node->done = done;
	new_node->link = NULL;
	
	if(pos == 0){
		new_node->link = head;
		head = new_node;
	}
	else if(pos > 0){
		prev = get_entry(pos - 1);
		if(prev != NULL)
			insert_next(prev, new_node);
		else{
			free(new_node);
			error("inserted position is error\n");
		}
	}
	else{
		free(new_node);
		error("inserted position is error\n");
	}
}

int delete_node(CElement deName, Element deMonth, Element deDay){
	Node *p, *removed;
	int n;
	int i = 0;
	for(p=head; p!=NULL; p=p->link){
		n = strcmp(p->name, deName);
		if((p->start.month == deMonth) && (p->start.day == deDay)){
			if(n == 0){
				removed = head;
				head = head->link;
				free(removed);
				return 0;
			}
		}
		if(p->link == NULL){
			if(p->start.month == deMonth && p->start.day == deDay){
				if(n == 0){
					removed = p;
					p = NULL;
					free(removed);
					return 0;
				}
			}
		}
		else if(p->link != NULL){
			if((p->link->start.month == deMonth) && (p->link->start.day == deDay)){
				n = strcmp(p->link->name, deName);
				if(n == 0){
					removed = p->link;
					p->link = p->link->link;
					free(removed);
					return 0;
				}
			}
		}
	}
	return 1;
}

Node* insertion_sort()
{
	Node *p = head, *tmp;
	
	Element startNow, startNext;
	while(p->link != NULL){
		startNow = p->start.month * 10000 + p->start.day * 100 + p->start.hour;
		startNext = p->link->start.month * 10000 + p->link->start.day * 100 + p->link->start.hour;
		if(startNow > startNext){
			tmp = p->link;
			p->link = p->link->link;
			tmp->link = head;
			head = p = tmp;
			continue;
		}
		p = p->link;
	}
	return p;
}

void save_file(){
	Node *p = insertion_sort(head);

	FILE *fp = fopen("schedule.txt", "w+");
	
	
	if(fp == NULL){
		printf("파일을 열 수 없습니다. 프로그램을 종료합니다.\n");
		return 1;
	}
	for(p=head; p!=NULL; p=p->link){
		fprintf(fp, "%d,%d,%d,%d,%d,%d,%s,%d\n", p->start.month, p->start.day, p->start.hour, p->end.month,
		p->end.day, p->end.hour, p->name, p->done);
	}
	
	fclose(fp);
}
// 날짜 전역 변수 
int year, month, wday, day;

// 좌표로 이동하는 함수 
void gotoxy(int x, int y)
{
	COORD pos ={x,y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int set_planner()
{
	init_list();
	time_t t;
	time(&t);
	struct tm *tm = localtime(&t);
	Node p;
	FILE *fp = fopen("schedule.txt", "r");
	
	if(fp == NULL){
		printf("파일을 열 수 없습니다. 프로그램을 종료합니다.\n");
		return 1;
	}
	
	fseek(fp, 0, SEEK_SET);
	
	while(!feof(fp)){		
		fscanf(fp, "%d,%d,%d,%d,%d,%d,%[^,],%d\n", &p.start.month, &p.start.day, &p.start.hour, &p.end.month, &p.end.day,
		&p.end.hour, p.name, &p.done);
		insert(size(), p.start, p.end, p.name, p.done);
	}
	fclose(fp);
	
	year = 1900 + tm->tm_year;
	month = 1 + tm->tm_mon;
	day = tm->tm_mday;
	wday = tm->tm_wday;
}

Element last_day(Element laMonth){
	if(laMonth < 1 || laMonth > 12)
		return -1;
	if(laMonth == 2){
		if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0){
			return 29;
		}
		else
			return 28;
	}
	else if(laMonth <= 7){
		if(laMonth % 2 == 0)
			return 30;
		else
			return 31;
	}
	else{
		if(laMonth % 2 == 0)
			return 31;
		else
			return 30;
	}
}

// 메인 화면 프레임을 그리는 함수 
void write_frame(int x, int length, int high)
{
	int y = 2;
	int i, j;
	
	gotoxy(x, y);
	for(i=0; i<2; i++){
		if(y == 2){printf("┌"); gotoxy(x+1, y);}
		if(y == high+2){printf("└"); gotoxy(x+1, y);}
		for(j=0; j<length; j++)
			printf("─");
		if(y == 2) {gotoxy(x+length+1, y); printf("┐");}
		if(y == high+2) {gotoxy(x+length+1, y); printf("┘");}
		gotoxy(x, y+=high);
	}
	
	for(i=3; i<high+2; i++){
		gotoxy(x, i);
		printf("│");
		gotoxy(x+length+1, i);
		printf("│");
	}
}

void daily_plan(int dpYear, int dpMonth, int dpDay)
{
	int n = 9, i;
	Node *p = head;

	daily_frame();
	gotoxy(83, 5);
	printf("%d년 %d월 %d일", dpYear, dpMonth, dpDay);
	for(p=head, i = 0; p!=NULL; p=p->link, i++)
	{	
		if(p->start.month > dpMonth || p->end.month < dpMonth || 
		(p->start.month == dpMonth && p->end.month == dpMonth && (p->start.day > dpDay || p->end.day < dpDay)))
			continue;
		gotoxy(63, n);
		if((p->start.hour != 0 && p->end.hour != 0) || !(p->start.hour == 0 && p->end.hour == 24)){
			if(p->start.month == dpMonth && p->end.month == dpMonth){
				if(p->start.day == dpDay && p->end.day == dpDay)
					printf("%02d:00~%02d:00", p->start.hour, p->end.hour);
				else if(p->start.day < dpDay && p->end.day == dpDay)
					printf("     ~%02d:00", p->end.hour);
				else if(p->start.day == dpDay && p->end.day > dpDay)
					printf("%02d:00~", p->start.hour);
			}
		}
		gotoxy(78, n);
			printf("%s", p->name);
			if(p->done == 1){
				gotoxy(110, n);
				printf("V");
			}
			n += 2;
	}
}

void daily_frame()
{
	int x = 61, y = 2;
	int i, j;
	
	write_frame(x, 52, 30);
	x = 63, y = 4;
	gotoxy(x, y);
	
	for(i=0; i<14; i++){
		for(j=0; j<51; j++)
			printf("─");
		gotoxy(x, y+=2);
	}
	gotoxy(x+51, 2);
	
	x = 61;
	int n = 1;
	for(i=5; i<32; i++){
		gotoxy(x+13, i);
		printf("│");
		if(i >= 9 && (i%2) != 0)
			printf(" %d", n++);
		gotoxy(x+44, i);
		printf("│");
	}
	
	gotoxy(x+21, 3);
	printf("오늘의 일정");
	gotoxy(x+6, 7);
	printf("시간");
	gotoxy(x+27, 7);
	printf("할 일"); 
	gotoxy(x+47, 7);
	printf("확인");
}

int add_plan_process(CElement name, TIME start, TIME end){
	int i, j;
	Element done = 0;
	
	insert(size(), start, end, name, done);
	save_file();
	gotoxy(12, 28);
	printf("추가되었습니다.");
	
	for(i=2; i<30; i++){
		for(j=0; j<52; j++){
			gotoxy(61+j, i);
			printf(" ");
		}
	}
	daily_plan(year, month, start.day);
}

void error_plan(char wrong[]){
	gotoxy(12, 28);
	printf("%s 잘못되었습니다.", wrong);
	gotoxy(12, 30);
	printf("▶ 이전");
	while(getch() != 13){} 
}

void add_plan_window(){
	fflush(stdin);
	CElement name;
	Element laDay;
	TIME start, end;
	
	system("cls");
	write_frame(9, 50, 30);
	daily_plan(year, month, day);
	gotoxy(31, 3);
	printf("일정 추가");
	gotoxy(12, 7);
	printf("일정 이름(최대12자) : ");
	scanf("%[^\n]", name);
	gotoxy(12, 9);
	printf("시작 월 : ");
	scanf("%02d", &start.month);
	gotoxy(12, 11);
	printf("시작 일 : ");
	scanf("%02d", &start.day);
	laDay = last_day(start.month);
	if(start.month >= 1 && start.month <= 12){
		if(start.day > laDay){
			error_plan("날짜가");
			return;
		}
	}
	else{
		error_plan("날짜가");
		return;
	}
	gotoxy(12, 13);
	printf("시작 시간 : ");
	scanf("%02d", &start.hour);
	if(start.hour < 0 && start.hour > 24){
		error_plan("시간이");
		return;
	}
	gotoxy(12, 15);
	printf("마감 월 : ");
	scanf("%02d", &end.month);
	gotoxy(12, 17);
	printf("마감 일 : ");
	scanf("%02d", &end.day);
	laDay = last_day(end.month);
	if(end.month >= start.month){
		if(end.month >= 1 && end.month <= 12){
			if(end.day > laDay){
				error_plan("날짜가");
				return;
			}
		}
	}
	else{
		error_plan("날짜가");
		return;
	}
	
	gotoxy(12, 19);
	printf("마감 시간 : ");
	scanf("%02d", &end.hour);
	if(start.month == end.month && start.day == end.day && start.hour > end.hour){
		error_plan("시간이");
		return;
	}
	else{
		if(end.hour < 0 && end.hour > 24){
			error_plan("시간이");
			return;
		}
	}
	
	add_plan_process(name, start, end);
	gotoxy(12, 30);
	printf("▶ 이전");
	while(getch() != 13){}
	main_window();
}

void search_plan()
{
	int key, y = 7;
	int sMonth, sDay;
	char name[25];
	Node *p;
	
	fflush(stdin);
	system("cls");
	write_frame(9, 50, 30);
	gotoxy(30, 3);
	printf("일정 검색");
	gotoxy(26, 7);
	printf("▶ 이름으로 검색");
	gotoxy(29, 12);
	printf("날짜로 검색");
	gotoxy(55, 33);
	printf("이전");
	gotoxy(0, 0);
	gotoxy(26, y);
	while(1){
		key = getch();
		if(key == 72){
			if(y == 33){
				gotoxy(48, y);
				printf("  ");
				gotoxy(26, y-=21);
				printf("▶");
			}
			else if(y == 12){
				gotoxy(26, y);
				printf("  ");
				gotoxy(26, y-=5);
				printf("▶");
			}
		}
		else if(key == 80){
			if(y == 7){
				gotoxy(26, y);
				printf("  ");
				gotoxy(26, y+=5);
				printf("▶");
			}
			else if(y == 12){
				gotoxy(26, y);
				printf("  ");
				gotoxy(51, y+=21);
				printf("▶");
			}
		}
		else if(key == 13){
			if(y == 7){
				gotoxy(27, 9);
				printf("▶ 일정이름 : ");
				scanf("%s", name);
				find_name(name);
				y = 33;
				gotoxy(51, y);
				printf("▶"); 
			}
			else if(y == 12){
				gotoxy(30, 14);
				printf("월 : ");
				scanf("%d", &sMonth);
				gotoxy(30, 16);
				printf("일 : ");
				scanf("%d", &sDay);
				daily_plan(year, sMonth, sDay);
				y = 33;
				gotoxy(51, y);
				printf("▶");
			}
			else if(y == 33)
				return;
		}
	}
}

void complete_plan()
{
	int i, j;
	CElement coName;
	Element coMonth, coDay;
	Node *p;
	
	fflush(stdin);
	gotoxy(11, 20);
	printf("완료할 일정의 이름 : ");
	scanf("%[^\n]", coName);
	gotoxy(11, 22);
	printf("완료할 일정의 월 : ");
	scanf("%d", &coMonth);
	gotoxy(11, 24);
	printf("완료할 일정의 일 : ");
	scanf("%d", &coDay);
	
	for(p=head; p!=NULL; p=p->link){
		if(p->start.month == coMonth && p->start.day == coDay && (strcmp(p->name, coName) == 0)){
			p->done = 1;
			gotoxy(11, 28);
			printf("완료되었습니다.");
			save_file();
			for(i=2; i<30; i++){
				for(j=0; j<52; j++){
					gotoxy(61+j, i);
					printf(" ");
				}
			}
			daily_plan(year, coMonth, coDay);
			break;
		}
	}
	if(p == NULL){
		gotoxy(11, 28);
		printf("이름 혹은 날짜를 정확히 입력해주세요");
	}
	
	gotoxy(55, 33);
	printf("▶ 이전");
	while(getch() != 13){}
	return;
}

void fix_plan(){
	int key, y = 7;
	 
	system("cls");
	write_frame(9, 50, 30);
	daily_plan(year, month, day);
	gotoxy(30, 3);
	printf("일정 수정 및 삭제");
	gotoxy(26, 7);
	printf("▶ 일정 완료");
	gotoxy(29, 12);
	printf("일정 삭제");
	gotoxy(55, 33);
	printf("이전");
	gotoxy(0, 0);
	gotoxy(26, y);
	while(1){
		key = getch();
		if(key == 72){
			if(y == 33){
				gotoxy(48, y);
				printf("  ");
				gotoxy(26, y-=21);
				printf("▶");
			}
			else if(y == 12){
				gotoxy(26, y);
				printf("  ");
				gotoxy(26, y-=5);
				printf("▶");
			}
		}
		else if(key == 80){
			if(y == 7){
				gotoxy(26, y);
				printf("  ");
				gotoxy(26, y+=5);
				printf("▶");
			}
			else if(y == 12){
				gotoxy(26, y);
				printf("  ");
				gotoxy(51, y+=21);
				printf("▶");
			}
		}
		else if(key == 13){
			if(y == 7){
				complete_plan();
				return;
			}
			else if(y == 12){
				delete_plan();
				return;
			}
			else if(y == 33)
				return;
		}
	}
}
void delete_plan(){
	int i, j;
	CElement name;
	Element deMonth, deDay;
	int deleted = 0;
	
	fflush(stdin); 
	write_frame(9, 50, 30);
	daily_plan(year, month, day);
	gotoxy(30, 3);
	printf("일정 삭제");
	gotoxy(11, 20);
	printf("삭제할 일정의 이름 : ");
	scanf("%[^\n]", name);
	gotoxy(11, 22);
	printf("삭제할 일정의 월 : ");
	scanf("%d", &deMonth);
	gotoxy(11, 24);
	printf("삭제할 일정의 일 : ");
	scanf("%d", &deDay);
	
	deleted = delete_node(name, deMonth, deDay);
	if(deleted == 0){
		gotoxy(11, 28);
		printf("삭제되었습니다.");
		save_file();
		for(i=2; i<30; i++){
			for(j=0; j<52; j++){
				gotoxy(61+j, i);
				printf(" ");
			}
		}
		daily_plan(year, deMonth, deDay);
	}
	else{
		gotoxy(11, 28);
		printf("이름 혹은 날짜를 정확히 입력해주세요");
	}
	gotoxy(55, 33);
	printf("▶ 이전");
	while(getch() != 13){}
	return;
}

void weekly_frame(){
	int x = 9, y = 2;
	int i, j;
	char months[15] = {"일월화수목금토"};
	
	system("cls");
	write_frame(x, 167, 23);
	for(i=0; i<2; i++){
		gotoxy(x+2, y+=2);
		for(j=0; j<166; j++)
			printf("─");
	}
	for(i=0; i<14; i+=2){
		gotoxy(x+12, 5);
		printf("%c%c", months[i], months[i+1]);
		x += 24;
		for(j=5; j<25; j++){
			gotoxy(x, j);
			printf("│");
		}
	}
	gotoxy(89, 3);
	printf("이번주 일정");
}

void weekly_plan(){
	int i, key, x = 11;
	char ch[3];
	Element laDay, wpMonth = month, wpDay = day - wday;
	Node* p = head;
	weekly_frame();
	
	if(wpDay < 1){
		wpMonth--;
		laDay = last_day(wpMonth);
		wpDay += laDay;
	}
	
	for(i=0; i<7; i++){
		laDay = last_day(wpMonth);
		if(wpDay > laDay){
			wpMonth++;
			wpDay = 1;
		}
		print_day(0, wpMonth, wpDay++, x, 7);
		x += 24;
	}
	gotoxy(150, 26);
	printf("▶ 이전");
	gotoxy(164, 26);
	printf("요일 선택");
	x = 150;
	gotoxy(x, 26);
	while(1){
		fflush(stdin);
		key = getch();
		if(key == 77){
			if(x != 160){
				gotoxy(x, 26);
				printf("  ");
				gotoxy(x += 10, 26);
				printf("▶");
			}
		}
		else if(key == 75){
			if(x != 150){
				gotoxy(x, 26);
				printf("  ");
				gotoxy(x -= 10, 26);
				printf("▶");
			}
		}
		else if(key == 13){
			if(x == 150)
				return;
			else{
				gotoxy(x, 28);
				printf("요일: ");
				scanf("%s", ch);
				wpDay -= 7;
				if(wpDay < 1){
					wpMonth--;
					laDay = last_day(wpMonth);
					wpDay = wpDay + laDay;
				}
				if(strcmp(ch, "월") == 0)
					wpDay += 1;
				else if(strcmp(ch, "화") == 0)
					wpDay += 2;
				else if(strcmp(ch, "수") == 0)
					wpDay += 3;
				else if(strcmp(ch, "목") == 0)
					wpDay += 4;
				else if(strcmp(ch, "금") == 0)
					wpDay += 5;
				else if(strcmp(ch, "토") == 0)
					wpDay += 6;
				else if(strcmp(ch, "일") == 0){}
				else{
					printf("날짜가 잘못되었습니다");
					return;
				}
				if(wpDay > laDay){
					wpMonth++;
					wpDay -= laDay;
				}
				system("cls");
				daily_plan(year, wpMonth, wpDay);
				gotoxy(107, 33);
				printf("▶ 이전");
				while(getch() != 13){}
				return;
			}
		}
	}
}

void monthly_frame(){
	int i, j;
	int x = 10, y = 4;
	char months[15] = {"일월화수목금토"};
	
	system("cls");
	for(i=0; i<3; i++){
		gotoxy(x, y);
		for(j=0; j<147; j++){
			printf("─");
		}
		y += 2;
	}
	y = 16;
	for(i=0; i<4; i++){
		gotoxy(x, y);
		for(j=0; j<147; j++){
			printf("─");
		}
		y += 8;
	}
	x = 9;
	for(i=0; i<6; i++){
		x += 21;
		for(j=9; j<48; j++){
			gotoxy(x, j);
			printf("│");
		}
	}
	write_frame(9, 147, 46);
	gotoxy(80, 3);
	printf("월간 일정");
	x = 9;
	for(i=0; i<14; i+=2){
		gotoxy(x+11, 7);
		printf("%c%c", months[i], months[i+1]);
		x += 21;
	}
}

void monthly_plan(Element mYear, Element mMonth){
	Element lastDay, temMonth = month, mDay = day, mWday = wday;
	int i, j, x = 21, y = 9;
	Node *p;
	
	monthly_frame();
	gotoxy(79, 5);
	printf("%d년 %d월", mYear, mMonth);
	while(mMonth < temMonth){
		if(mDay == 1)
			mDay = last_day(--temMonth);
		else
			mDay--;
		if(mWday == 0)
			mWday = 6;
		else
			mWday--;
	}
	while(mMonth > temMonth){
		lastDay = last_day(temMonth);
		if(mDay >= lastDay){
			temMonth++;
			mDay = 1;
		}
		else
			mDay++;
		if(mWday == 6)
			mWday = 0;
		else
			mWday++;
	}
	
	lastDay = last_day(mMonth);
	for(i=mDay; i>1; i--){
		if(mWday == 0)
			mWday = 6;
		else
			mWday--;
	}
	for(i=1; i<=lastDay; i++){
		gotoxy(x*mWday + 20, y);
		printf("%d", i);
		print_day(1, mMonth, i, x*mWday + 11, y+1);
		/*
		for(p=head, j=1; p!=NULL; p=p->link){
			if(p->start.month == mMonth && p->start.day == i){
				gotoxy(x*mWday + 11, y+j);
				printf("%s", p->name);
				j++;
				if(j > 4){
					gotoxy(x*mWday + 19, y+j);
					printf("등");
					break;
				}
			}
		}
		*/
		mWday++;
		if(mWday > 6){
			y += 8;
			mWday = 0;
		}
	}
}

void period_frame(){
	system("cls");
	write_frame(9, 50, 30);
	gotoxy(30, 3);
	printf("기간별 일정");
	
	gotoxy(31, 13);
	printf("오늘의 일정");
	gotoxy(31, 16);
	printf("주간 일정");
	gotoxy(31, 19);
	printf("월간 일정");
	gotoxy(55, 33);
	printf(" 이전");
}

void monthly_select()
{
	gotoxy(103, 49);
	printf("이전");
	gotoxy(118, 49);
	printf("날짜 선택");
	gotoxy(134, 49);
	printf("이전 달");
	gotoxy(149, 49);
	printf("다음 달");
}

void period_plan(){
	int key, x, y = 13;
	Element peDay, peMonth = month, peYear = year;
	
	period_frame();
	gotoxy(0, 0);
	gotoxy(28, y);
	printf("▶");
	while(1){
		key = getch();
		if(key == 72){
			if(y == 33){
				gotoxy(53, y);
				printf("  ");
				gotoxy(28, y-=14);
				printf("▶");
			}
			else if(y != 13){
				gotoxy(28, y);
				printf("  ");
				gotoxy(28, y-=3);
				printf("▶");
			}
				
		}
		else if(key == 80){
			if(y == 19){
				gotoxy(28, y);
				printf("  ");
				gotoxy(53, y+=14);
				printf("▶");
			}
			else if(y != 33){
				gotoxy(28, y);
				printf("  ");
				gotoxy(28, y+=3);
				printf("▶");
			}
		}
		else if(key == 13){
			if(y == 13)
				daily_plan(year, month, day);
			else if(y == 16){
				weekly_plan();
				period_frame();
				gotoxy(28, y);
				printf("▶");
			}
			else if(y == 19){
				monthly_plan(year, month);
				monthly_select();
				fflush(stdin);
				x = 100;
				gotoxy(x, 49);
				printf("▶");
				while(1){
					key = getch();
					if(key == 77){
						if(x != 145){
							gotoxy(x, 49);
							printf("  ");
							x += 15;
							gotoxy(x, 49);
							printf("▶");
						}
					}
					else if(key == 75){
						if(x != 100){
							gotoxy(x, 49);
							printf("  ");
							x -= 15;
							gotoxy(x, 49);
							printf("▶");
						}
					}
					else if(key == 13){
						if(x == 100){
							period_frame();
							break;
						}
						else if(x == 115){
							gotoxy(x, 50);
							printf("날짜(일) : ");
							scanf("%d", &peDay);
							system("cls");
							daily_plan(year, peMonth, peDay);
							gotoxy(107, 33);
							printf("▶ 이전");
							while(getch() != 13){}
							period_frame();
							break;
						}
						else if(x == 130){
							system("cls");
							if(peMonth == 1){
								peYear--;
								peMonth = 12;
							}
							else
								peMonth--;
							monthly_plan(peYear, peMonth);
							monthly_select();
							gotoxy(x, 49);
							printf("▶");
						}
						else if(x == 145){
							system("cls");
							if(peMonth == 12){
								peYear++;
								peMonth = 1;
							}
							else
								peMonth++;
							monthly_plan(peYear, peMonth);
							monthly_select();
							gotoxy(x, 49);
							printf("▶");
						}
					}
				}
				period_frame();
				gotoxy(28, y);
				printf("▶");
			}
			else if(y == 33){
				return;
			}
		}
	}
}

void main_window(){
	
	system("cls");
	write_frame(9, 50, 30);
	gotoxy(24, 3);
	printf("대학생을 위한 학습 플래너");
	// 버전
	gotoxy(50, 5);
	printf("v1.1");
	
	gotoxy(31, 11);
	printf("일정 추가");
	gotoxy(31, 14);
	printf("일정 검색");
	gotoxy(31, 17);
	printf("일정 수정 및 삭제");
	gotoxy(31, 20);
	printf("기간별 일정");
	gotoxy(31, 23);
	printf("종료");
}

void main_plan(){
	int key, y = 11;
	
	main_window();
	gotoxy(0, 0);
	gotoxy(28, 11);
	printf("▶");
	gotoxy(29, y);
	
	while(1){
		key = getch();
		
		if(key == 72){
			if(y != 11)
			{
				gotoxy(28, y);
				printf("  ");
				gotoxy(28, y-=3);
				printf("▶");
			}
		}
		else if(key == 80){
			if(y != 23)
			{
				gotoxy(28, y);
				printf("  ");
				gotoxy(28, y+=3);
				printf("▶");
			}
		}
		else if(key == 13){
			if(y == 11){
				add_plan_window();
				main_window();
				gotoxy(0, 0);
				gotoxy(28, y);
				printf("▶");
			}
			else if(y == 14){
				search_plan();
				gotoxy(51, 33);
				printf("▶");
				main_window();
				gotoxy(0, 0);
				gotoxy(28, y);
				printf("▶");
			}
			else if(y == 17){
				fix_plan();
				main_window();
				gotoxy(0, 0);
				gotoxy(28, y);
				printf("▶"); 
			}
			else if(y == 20){
				period_plan();
				main_window();
				gotoxy(0, 0);
				gotoxy(28, y);
				printf("▶"); 
			}
			else if(y == 23)
				exit(0);
		}
	}
}

int main(void)
{
	system("mode con cols=250 lines=80");
	set_planner();
	main_plan();
}
