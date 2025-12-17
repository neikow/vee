#ifndef VEE_STATISTICS_H
#define VEE_STATISTICS_H
#include "../../editor.h"


/** Class representing the Statistics interface component.
 */
namespace Editor::UI {
    class Statistics {
        /** Draws the Renderer Statistics UI component.
         *
         * @param renderer
         */
        static void DrawRendererStatistics(const std::shared_ptr<AbstractRenderer> &renderer);

    public:
        /** Draws the Statistics window.
         *
         * @param title The title of the window.
         * @param editor Pointer to the VeeEditor instance.
         */
        static void Draw(const char *title, const VeeEditor *editor);
    };
}


#endif //VEE_STATISTICS_H
