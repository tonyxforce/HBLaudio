#pragma once

void log(String t)
{
    Serial.print(t);
    SerialBT.print(t);
}
void log(uint8_t t)
{
    Serial.print(t);
    SerialBT.print(t);
}
void log(int t)
{
    Serial.print(t);
    SerialBT.print(t);
}
void logln(String t)
{
    log(t + "\n");
}
void logln(uint8_t t)
{
    log(t);
    log("\n");
}
void logf(const char *t, const uint8_t *z)
{
    Serial.printf(t, z);
    SerialBT.printf(t, z);
}