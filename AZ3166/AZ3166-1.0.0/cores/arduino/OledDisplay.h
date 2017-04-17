/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__

#ifdef __cplusplus

class OLEDDisplay
{
public:
    OLEDDisplay();
    ~OLEDDisplay();
    
    void init();
    void clean();
    
    virtual uint8_t print(const char *s, bool wrap = false);
    virtual uint8_t print(uint8_t line, const char *s, bool wrap = false);
        
private:
    uint8_t println(uint8_t line, const char *s, uint8_t len, bool wrap);
};

#endif  // __cplusplus
#endif  // __OLED_DISPLAY_H__
