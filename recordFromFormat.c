/*
 * This file implements two functions that read XML and binary information from a buffer,
 * respectively, and return pointers to Record or NULL.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "recordFromFormat.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>

char* get_value(char *buffer, char *needle);
uint8_t get_number_uint8(char *buffer, char *needle);
int get_courseCode(char *buffer, char *needle);
uint32_t get_number_uint32(char *buffer, char *needle);
Grade get_grade(char *buffer, char *needle);


Record* XMLtoRecord( char* buffer, int bufSize, int* bytesread )
{
    struct Record *theRecord = newRecord();
    initRecord(theRecord);
    char *source = get_value(buffer, "source");
    if (source != NULL) {
        setSource(theRecord, source[0]);
        free(source);
    } else {
        theRecord->has_source = false;
    }

    char *dest = get_value(buffer, "dest");
    if (dest != NULL) {
        setDest(theRecord, dest[0]);
        free(dest);
    } else {
        deleteRecord(theRecord);
        return NULL;
    }

    char *username = get_value(buffer, "username");
    if (username != NULL) {
        setUsername(theRecord, username);
    } else {
        theRecord->has_username = false;
    }

    uint32_t id = get_number_uint32(buffer, "id");
    if (id > 0) {
        setId(theRecord, id);
    } else {
        theRecord->has_id = false;
    }

    uint32_t group = get_number_uint32(buffer, "group");
    if (group > 0) {
        setGroup(theRecord, group);
    } else {
        theRecord->has_group = false;
    }

    uint8_t semester = get_number_uint8(buffer, "semester");
    if (semester > 0) {
        setSemester(theRecord, semester);
    } else {
        theRecord->has_semester = false;
    }

    Grade grade = get_grade(buffer, "grade");
    if (grade != -1) {
        setGrade(theRecord, grade);
    } else {
        theRecord->has_grade = false;
    }

    // Maybe a bit questionable to use pointer arethmics, but it seems like a simple solution to get next course.
    char *courses_buffer = strstr(buffer, "course");
    int course_code = 0;
    int amount_courses = 0;
    while(courses_buffer != NULL) {
        course_code = get_courseCode(courses_buffer, "course=");
        if (course_code == -1) {
            break;
        }
        courses_buffer = strstr(courses_buffer+1, "course");
        setCourse(theRecord, course_code);
        amount_courses++;
    }
    if (amount_courses == 0) {
        theRecord->has_courses = false;
    }

    return theRecord;
}

Record* BinaryToRecord( char* buffer, int bufSize, int* bytesread ) {
    struct Record* theRecord = newRecord();
    initRecord(theRecord);
    int read_bytes = 0;

    char flagByte = buffer[0];

    read_bytes++;

    if (flagByte & FLAG_SRC) {
        setSource(theRecord, buffer[read_bytes]);
        read_bytes++;
    } else {
        theRecord->has_source = false;
    }

    if (flagByte & FLAG_DST) {
        setDest(theRecord, buffer[read_bytes]);
        read_bytes++;
    } else {
        theRecord->has_dest = false;
    }

    if (flagByte & FLAG_USERNAME) {
        uint32_t length;
        memcpy(&length, &buffer[read_bytes], sizeof(uint32_t));
        length = ntohl(length);
        read_bytes += sizeof(uint32_t);

        char* username = malloc(length + 1);
        memcpy(username, &buffer[read_bytes], length);
        username[length] = '\0';
        setUsername(theRecord, username);
        read_bytes += length;
    } else {
        theRecord->has_username = false;
    }

    if (flagByte & FLAG_ID) {
        uint32_t ID;
        memcpy(&ID, &buffer[read_bytes], sizeof(uint32_t));
        ID = ntohl(ID);
        setId(theRecord, ID);
        read_bytes += sizeof(uint32_t);
    } else {
        theRecord->has_id = false;
    }

    if (flagByte & FLAG_GROUP) {
        uint32_t group;
        memcpy(&group, &buffer[read_bytes], sizeof(uint32_t));
        group = ntohl(group);
        setGroup(theRecord, group);
        read_bytes += sizeof(uint32_t);
    } else {
        theRecord->has_group = false;
    }

    if (flagByte & FLAG_SEMESTER) {
        uint8_t semester = buffer[read_bytes];
        setSemester(theRecord, semester);
        read_bytes++;
    } else {
        theRecord->has_semester = false;
    }

    if (flagByte & FLAG_GRADE) {
        uint8_t grade = buffer[read_bytes];
        if (grade == 0) {
            setGrade(theRecord, Grade_None);
        } else if (grade == 1) {
            setGrade(theRecord, Grade_Bachelor);
        } else if (grade == 2) {
            setGrade(theRecord, Grade_Master);
        } else if (grade == 3) {
            setGrade(theRecord, Grade_PhD);
        }
        read_bytes++;
    } else {
        theRecord->has_grade = false;
    }

    if (flagByte & FLAG_COURSES) {
        uint16_t course_flags;
        memcpy(&course_flags, &buffer[read_bytes], sizeof(uint16_t));
        read_bytes += sizeof(uint16_t);
        if (course_flags & Course_IN1000) {
            setCourse(theRecord, Course_IN1000);
        }
        if (course_flags & Course_IN1010) {
            setCourse(theRecord, Course_IN1010);
        }
        if (course_flags & Course_IN1020) {
            setCourse(theRecord, Course_IN1020);
        }
        if (course_flags & Course_IN1030) {
            setCourse(theRecord, Course_IN1030);
        }
        if (course_flags & Course_IN1050) {
            setCourse(theRecord, Course_IN1050);
        }
        if (course_flags & Course_IN1060) {
            setCourse(theRecord, Course_IN1060);
        }
        if (course_flags & Course_IN1080) {
            setCourse(theRecord, Course_IN1080);
        }
        if (course_flags & Course_IN1140) {
            setCourse(theRecord, Course_IN1140);
        }
        if (course_flags & Course_IN1150) {
            setCourse(theRecord, Course_IN1150);
        }
        if (course_flags & Course_IN1900) {
            setCourse(theRecord, Course_IN1900);
        }
        if (course_flags & Course_IN1910) {
            setCourse(theRecord, Course_IN1910);
        }
    } else {
        theRecord->has_courses = false;
    }
    *bytesread = read_bytes;
    return theRecord;
}


char* get_value(char *buffer, char *needle) {
    char *substring = strstr(buffer, needle);
    if (substring == NULL) {
        return NULL;
    }
    int needle_length = strlen(needle);
    int value_length = 0;
    while(1) {
        if(substring[(needle_length + 2) + value_length] != '"') {
            value_length++;
        } else {
            break;
        }
    }
    char *value = malloc(value_length + 1);

    strncpy(value, &substring[(needle_length + 2)], value_length);
    if(value_length > 1) {
        value[value_length] = '\0';
    }
    return value;
}

uint8_t get_number_uint8(char *buffer, char *needle) {
    char* value = get_value(buffer, needle);
    if(value == NULL) {
        return 0;
    }
    uint8_t number = atoi(value);
    free(value);
    return number;
}

int get_courseCode(char *buffer, char *needle) {
    // Very uhm, bad way to fix.
    char *course_code = malloc(7);
    strcpy(course_code, "I");
    char *value = get_value(buffer, needle);
    if (value != NULL) {
        strcat(course_code, value);  // Concatenate the value to the course_code string
    }
    if (course_code == NULL) {
        return -1;
    }
    int course = -1;
    if (strcmp("IN1000", course_code) == 0) {
        course = Course_IN1000;
    } else if (strcmp("IN1010", course_code) == 0) {
        course = Course_IN1010;
    } else if (strcmp("IN1020", course_code) == 0) {
        course = Course_IN1020;
    } else if (strcmp("IN1030", course_code) == 0) {
        course = Course_IN1030;
    } else if (strcmp("IN1050", course_code) == 0) {
        course = Course_IN1050;
    } else if (strcmp("IN1060", course_code) == 0) {
        course = Course_IN1060;
    } else if (strcmp("IN1080", course_code) == 0) {
        course = Course_IN1080;
    } else if (strcmp("IN1140", course_code) == 0) {
        course = Course_IN1140;
    } else if (strcmp("IN1150", course_code) == 0) {
        course = Course_IN1150;
    } else if (strcmp("IN1900", course_code) == 0) {
        course = Course_IN1900;
    } else if (strcmp("IN1910", course_code) == 0) {
        course = Course_IN1910;
    }

    free(course_code);
    free(value);
    return course;
}

// I read that atoi is not to be used for bigger integers, hence the strtoul
uint32_t get_number_uint32(char *buffer, char *needle) {
    char* value = get_value(buffer, needle);
    if(value == NULL) {
        return 0;
    }
    uint32_t number = strtoul(value, NULL, 10);
    free(value);
    return number;
}

Grade get_grade(char *buffer, char *needle) {
    char* value = get_value(buffer, needle);
    if (value == NULL) {
        return -1;
    }
    if (strcmp(value, "PhD") == 0) {
        free(value);
        return Grade_PhD;
    } else if (strcmp(value, "Master") == 0) {
        free(value);
        return Grade_Master;
    } else if (strcmp(value, "Bachelor") == 0) {
        free(value);
        return Grade_Bachelor;
    } else if (strcmp(value, "None") == 0) {
        free(value);
        return Grade_None;
    }
    free(value);
    return 0;
}