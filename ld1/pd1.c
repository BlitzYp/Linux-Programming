#include <stdio.h>

int sv_garums(char *virkne)
{
    int c = 0;
    for (; *virkne != '\0'; virkne++) c++;
    return c;
}

void sv_kopet(char *no, char *uz)
{
    for (; *no != '\0'; no++, uz++) *uz = *no;
    *uz = '\0';
}

int sv_meklet(char *kur, char *ko)
{
    if (*ko == '\0') return 0;
    for (int i = 0; *kur != '\0'; kur++, i++){
        if (*kur == ko[0]) {
            int f = 1;
            for (int j = 0; ko[j] != '\0'; j++) {
                if (kur[j] == '\0' || ko[j] != kur[j]) {
                    f = 0;
                    break;
                }
            }
            if (f) return i;
        }
    }
    return -1;
}

void sv_apgriezt(char *virkne)
{
    char *beg, *end;
    int l = sv_garums(virkne);
    if (l <= 1) return;
    beg = virkne;
    end = virkne + l - 1;
    if (beg == end) return;
    for (; beg < end; beg++, end--) {
        char temp = *beg;
        *beg = *end;
        *end = temp;
    }
}

void sv_apgriezt_int(char *beg, char *end)
{
    for (; beg < end; beg++, end--) {
        char temp = *beg;
        *beg = *end;
        *end = temp;
    }
}

void sv_vapgriezt(char *virkne)
{
    int l = sv_garums(virkne);
    if (l <= 1) return;
    char *beg = virkne;
    char *end = virkne + l - 1;
    // Ignorējam atstarpes sākumā un beigās 
    for (; end >= virkne && *end == ' '; end--);
    if (end < virkne) return;
    for (; beg < end && *beg == ' '; beg++);
    sv_apgriezt_int(beg, end);
    for (char *p = beg; p <= end; p++) {
        if (*p == ' ') continue;
        beg = p;
        for (; p <= end && *p != ' '; p++);
        char *e = p - 1;
        if (beg < e) sv_apgriezt_int(beg, e);
    }
}

int main(void)
{
    char buferis[100];

    printf("Tests uzdevumam PD1.1:\n");
    printf("%d\n", sv_garums("hello world")); // 11
    printf("%d\n", sv_garums("123"));         // 3
    printf("%d\n", sv_garums(""));            // 0

    printf("Tests uzdevumam PD1.2:\n");
    sv_kopet("Liels bet nav", buferis);
    printf("%s\n", buferis);

    printf("Tests uzdevumam PD1.3:\n");
    printf("%d\n", sv_meklet("hello world", "world")); // 6
    printf("%d\n", sv_meklet("abcdefg", "de"));        // 3
    printf("%d\n", sv_meklet("abc", "nav un garaks")); // -1
    printf("%d\n", sv_meklet("strstr", ""));           // 0
    printf("%d\n", sv_meklet("strstr", "str"));        // 0

    printf("Tests uzdevumam PD1.4:\n");
    sv_apgriezt(buferis);
    printf("%s\n", buferis); // van tab sieL
    sv_apgriezt(buferis);
    printf("%s\n", buferis); // Liels bet nav

    printf("Tests uzdevumam PD1.5:\n");
    sv_vapgriezt(buferis);
    printf("%s\n", buferis);

    char buferis2[100];
    sv_kopet("a", buferis2);
    printf("%s\n", buferis2); // a
    sv_vapgriezt(buferis2);
    printf("%s\n", buferis2); // a
    sv_kopet("Sveika pasaulite", buferis2);
    sv_vapgriezt(buferis2);
    printf("%s\n", buferis2); // pasaulite Sveika
    sv_kopet("  Kaut  kas liels", buferis2);
    sv_vapgriezt(buferis2);
    printf("%s\n", buferis2); // "  liels kas   Kaut"

    return 0;
}