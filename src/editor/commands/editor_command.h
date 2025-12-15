#ifndef VEE_EDITOR_COMMANDS_EDITOR_COMMAND_HPP
#define VEE_EDITOR_COMMANDS_EDITOR_COMMAND_HPP

namespace Editor::Command {
    /**
      * Interface for editor commands implementing the Command Pattern.
      *
      */
    class IEditorCommand {
    public:
        virtual ~IEditorCommand() = default;

        /**
         * Execute the command.
         */
        virtual void Execute() = 0;
    };
}

#endif
