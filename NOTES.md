- [ ] return null instead of exit in parse.c
   * save an error message with ASSERT macro?
     * maybe need an "UNASSERT" macro which will reset the error message when we don't plan on using it?
   * propogate down call stack and then error out
   * pros: makes it easier to try_parse something
   * cons: error handling is weird now
   
   
- codegen
   - a function is
      - code
      - params
      - position in memory
   - for codegen
      - code
         - gotta understand the current scope to do shit
      - params
         - push onto stack, access via stack pointer
         - each variable therefore has an index on the stack
         
- Demacro instr.c
